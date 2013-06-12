#pragma once

#include "gpSolver.h"
#include "GadgetronException.h"
#include "real_utilities.h"
#include "complext.h"
#include <vector>
#include <iostream>

namespace Gadgetron{

  /* Using adaptive step size from Zhou et al, 2006, Computational Optimization and Applications,
   * DOI: 10.1007/s10589-006-6446-0
   */

  template <class ARRAY_TYPE> class gpBBSolver : public gpSolver<ARRAY_TYPE> 
  {
  protected:	
    typedef typename ARRAY_TYPE::element_type ELEMENT_TYPE;
    typedef typename realType<ELEMENT_TYPE>::Type REAL;
  
  public:

    gpBBSolver(): gpSolver<ARRAY_TYPE>() {    
      iterations_ = 10;
      tc_tolerance_ = (REAL)1e-6;
      non_negativity_constraint_=false;
      dump_residual = false;
      threshold= REAL(1e-8);
    }

    virtual ~gpBBSolver(){}

    boost::shared_ptr<ARRAY_TYPE> solve(ARRAY_TYPE* in)
    {      
      if( this->encoding_operator_.get() == 0 ){
	BOOST_THROW_EXCEPTION(runtime_error("Error: gpBBSolver::compute_rhs : no encoding operator is set" ));
      }

      // Get image space dimensions from the encoding operator
      //

      boost::shared_ptr< std::vector<unsigned int> > image_dims = this->encoding_operator_->get_domain_dimensions();
      if( image_dims->size() == 0 ){
	BOOST_THROW_EXCEPTION(runtime_error("Error: gpBBSolver::compute_rhs : encoding operator has not set domain dimension" ));
      }

      ARRAY_TYPE * x = new ARRAY_TYPE;
      x->create(image_dims.get());

      ARRAY_TYPE x_old(image_dims.get());

      ARRAY_TYPE * g = new ARRAY_TYPE;
      g->create(image_dims.get());
      ARRAY_TYPE *  g_old = new ARRAY_TYPE;
      g_old->create(image_dims.get());

      if (this->x0_.get()){
	*x = *(this->x0_.get());
      } else  {
	clear(x);
      }

      ARRAY_TYPE encoding_space;
      REAL reg_res,data_res;
      encoding_space.create(in->get_dimensions().get());
      if( this->output_mode_ >= solver<ARRAY_TYPE,ARRAY_TYPE>::OUTPUT_VERBOSE ){
	std::cout << "Iterating..." << std::endl;
      }
      for (int i = 0; i < iterations_; i++){
	if ((i==0) && (!this->x0_.get())){
	  clear(g);

	  this->encoding_operator_->mult_MH(in,g);

	  *g *=  -this->encoding_operator_->get_weight();
	  data_res = dot(in,in);
	  reg_res=REAL(0);
	} else {
	  this->encoding_operator_->mult_M(x,&encoding_space);
	  axpy(REAL(-1),in,&encoding_space);
	  data_res = dot(&encoding_space,&encoding_space);
	  this->encoding_operator_->mult_MH(&encoding_space,g);
	  *g *=  this->encoding_operator_->get_weight();
	}
	
	this->add_gradient(x,g); // Adds the gradient from all the regularization operators
	
	if( this->output_mode_ >= solver<ARRAY_TYPE,ARRAY_TYPE>::OUTPUT_VERBOSE ){
	  std::cout << "Data residual: " << data_res << std::endl;
	}

	if (non_negativity_constraint_) solver_non_negativity_filter(x,g);
	REAL nabla;
	if (i==0){
	  ARRAY_TYPE tmp_encoding = *in;
	  this->encoding_operator_->mult_M(g,&tmp_encoding);
	  if (this->x0_.get()){
	    nabla = dot(&encoding_space,&tmp_encoding)/dot(&tmp_encoding,&tmp_encoding);
	  } else {
	    nabla = -dot(in,&tmp_encoding)/dot(&tmp_encoding,&tmp_encoding);
	  }	  
	} else {
	  x_old -= *x;
	  *g_old -= *g;
	  REAL xx = dot(&x_old,&x_old);
	  REAL gx = dot(g_old,&x_old);

	  REAL nabla1 = xx/gx;

	  /* This is the code that enables the adaptive step size.
	     REAL gg = dot(g_old,&x_old);
	     REAL nabla2 = gx/gg;
	     if ((nabla2/nabla1) < 0.5) nabla = nabla2;
	     else nabla = nabla1;*/
	  nabla = nabla1;
	}

	ARRAY_TYPE * tmp;
	tmp=g_old;
	g_old=g;
	g=tmp;
	
	x_old = *x;
	REAL grad_norm = nrm2(g_old);

	if( this->output_mode_ >= solver<ARRAY_TYPE,ARRAY_TYPE>::OUTPUT_VERBOSE ){
	  std::cout << "Iteration " <<i << ". Gradient norm: " <<  grad_norm << std::endl;
	}
	iteration_callback(x,i,data_res,reg_res);
	axpy(-nabla,g_old,x);
	if (non_negativity_constraint_) clamp_min(x,ELEMENT_TYPE(0));
	if (grad_norm < tc_tolerance_)  break;
      }
      delete g,g_old;
      
      return boost::shared_ptr<ARRAY_TYPE>(x);
    }
    
    // Set preconditioner
    //
    /*virtual void set_preconditioner( boost::shared_ptr< cgPreconditioner<ARRAY_TYPE> > precond ) {
      precond_ = precond;
      }*/

    // Set/get maximally allowed number of iterations
    //
    virtual void set_max_iterations( unsigned int iterations ) { iterations_ = iterations; }
    virtual unsigned int get_max_iterations() { return iterations_; }

    // Set/get tolerance threshold for termination criterium
    //
    virtual void set_tc_tolerance( REAL tolerance ) { tc_tolerance_ = tolerance; }
    virtual REAL get_tc_tolerance() { return tc_tolerance_; }

    virtual void set_non_negativity_constraint(bool non_negativity_constraint){
      non_negativity_constraint_=non_negativity_constraint;
    }

    virtual void set_dump_residual(bool dump_res){
      dump_residual = dump_res;
    }
    
  protected:
    typedef typename std::vector<boost::shared_ptr<linearOperator<ARRAY_TYPE> > >::iterator  csIterator;
    typedef typename std::vector< std::vector<boost::shared_ptr<linearOperator<ARRAY_TYPE> > > >::iterator csGroupIterator;
    
    virtual void solver_non_negativity_filter(ARRAY_TYPE*,ARRAY_TYPE*)=0;
    virtual void iteration_callback(ARRAY_TYPE*,int i,REAL,REAL){};

  protected:

    // Preconditioner
    //boost::shared_ptr< cgPreconditioner<ARRAY_TYPE> > precond_;
    // Maximum number of iterations
    unsigned int iterations_;
    bool non_negativity_constraint_;
    REAL tc_tolerance_;
    REAL threshold;
    bool dump_residual;
  };
}
