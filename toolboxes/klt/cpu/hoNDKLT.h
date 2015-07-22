/** \file   hoNDKLT.h
    \brief  Implementation Karhunen-Loeve transform (KLT) functionalities
    \author Hui Xue
*/

#ifndef hoNDKLT_H
#define hoNDKLT_H

#include "hoNDArray.h"
#include "cpuklt_export.h"

#ifdef USE_OMP
    #include "omp.h"
#endif // USE_OMP

namespace Gadgetron{

    template <typename T> class EXPORTCPUKLT hoNDKLT
    {
    public:

        typedef typename realType<T>::Type value_type;
        typedef hoNDKLT<T> Self;

        hoNDKLT();
        hoNDKLT(const hoNDArray<T>& data, size_t dim, size_t output_length);
        hoNDKLT(const hoNDArray<T>& data, size_t dim, value_type thres);
        hoNDKLT(const Self& v);

        virtual ~hoNDKLT();

        Self& operator=(const Self& v);

        /// Calculates the KLT transform matrix or basis functions
        /// this function will compute the KLT transformation matrix and its inverse
        /// M : [data.get_size(dim) output_length]
        /// output_length == 0 means keep all modes
        void prepare(const hoNDArray<T>& data, size_t dim = 0, size_t output_length = 0);
        /// the output length will be determined by thres; the minimal eigen value kept is >= (max eigen value * thres)
        void prepare(const hoNDArray<T>& data, size_t dim = 0, value_type thres=(value_type)0.001);
        /// set the eigen vector matrix
        void prepare(const hoNDArray<T>& V, const hoNDArray<T>& E, size_t output_length = 0);
        /// set the KL transformation matrix
        void prepare(const hoNDArray<T>& M);

        /// Applies the transform
        /// The input array size must meet in.get_size(dim) == M.get_size(0)
        /// out array will have out.get_size(dim)==out_length
        void transform(const hoNDArray<T>& in, hoNDArray<T>& out, size_t dim = 0);

        /// return M.get_size(0), the length of tranformed dimension
        size_t transform_length();
        /// return M.get_size(1), the length of output dimension
        size_t output_length();

        /// set output length, '0' means keep all modes
        void output_length(size_t length = 0);

        /// get the KL transformation matrix
        void KL_transformation(hoNDArray<T>& M) const;
        /// get the eigen vector matrix
        void eigen_vector(hoNDArray<T>& V) const;
        /// get the eigen values
        void eigen_value(hoNDArray<T>& E) const;

    protected:

        /// KL tranformation matrix
        hoNDArray<T> M_;
        /// eigen vector matrix
        hoNDArray<T> V_;
        /// eigen value array, ascending order
        hoNDArray<T> E_;
        /// length of output dimension
        size_t output_length_;

        /// compute eigen vector and values
        void compute_eigen_vector(const hoNDArray<T>& data, hoNDArray<T>& V, hoNDArray<T>& E);
    };
}

#endif //hoNDKLT_H
