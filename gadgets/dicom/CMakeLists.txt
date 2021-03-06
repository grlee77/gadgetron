# DCMTK-necessary preprocessor flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_CONFIG_H -D_REENTRANT -D_OSF_SOURCE")

set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost COMPONENTS date_time REQUIRED)

if(WIN32)
  link_directories(${Boost_LIBRARY_DIRS})
endif(WIN32)

if (WIN32)
    set(GT_DICOM_LIBRARIES ${DCMTK_dcmdata_LIBRARY} ${DCMTK_oflog_LIBRARY} ${DCMTK_ofstd_LIBRARY})
else (WIN32)
    set(GT_DICOM_LIBRARIES
        ${DCMTK_dcmdata_LIBRARY}
        ${DCMTK_oflog_LIBRARY}
        ${DCMTK_ofstd_LIBRARY}
        z m pthread)
    if (APPLE)
        list(APPEND GT_DICOM_LIBRARIES iconv)
    endif (APPLE)
endif (WIN32)

# sanity check:
#message("DCMTK ${DCMTK_HOME}")
#message("Include: ${DCMTK_INCLUDE_DIRS}")
#message("Libraries: ${GT_DICOM_LIBRARIES}")

include_directories(
        ${CMAKE_SOURCE_DIR}/gadgets/mri_core    # for GadgetIsmrmrdReadWrite.h
        ${DCMTK_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/toolboxes/mri/pmri/gpu
        ${CMAKE_SOURCE_DIR}/toolboxes/nfft/gpu
        ${CMAKE_SOURCE_DIR}/toolboxes/core
        ${CMAKE_SOURCE_DIR}/toolboxes/core/gpu
        ${CMAKE_SOURCE_DIR}/toolboxes/core/cpu
        ${CMAKE_SOURCE_DIR}/toolboxes/core/cpu/image
        ${CMAKE_SOURCE_DIR}/toolboxes/core/cpu/hostutils
        ${CMAKE_SOURCE_DIR}/toolboxes/core/cpu/math
        ${CMAKE_SOURCE_DIR}/toolboxes/core/cpu/image
        ${CMAKE_SOURCE_DIR}/toolboxes/core/cpu/algorithm
        ${CMAKE_SOURCE_DIR}/toolboxes/operators
        ${CMAKE_SOURCE_DIR}/toolboxes/operators/cpu
        ${CMAKE_SOURCE_DIR}/toolboxes/solvers
        ${CMAKE_SOURCE_DIR}/toolboxes/solvers/cpu
        ${CMAKE_SOURCE_DIR}/gadgets/core
        ${CMAKE_SOURCE_DIR}/toolboxes/core/cpu/math
        ${CMAKE_SOURCE_DIR}/toolboxes/registration/optical_flow
        ${CMAKE_SOURCE_DIR}/toolboxes/mri_core
        ${CMAKE_SOURCE_DIR}/toolboxes/gtplus
        ${CMAKE_SOURCE_DIR}/toolboxes/gtplus/util
        ${CMAKE_SOURCE_DIR}/toolboxes/gtplus/workflow
        ${CMAKE_SOURCE_DIR}/toolboxes/gtplus/algorithm
        ${CMAKE_SOURCE_DIR}/toolboxes/gtplus/solver
        ${CMAKE_SOURCE_DIR}/toolboxes/gadgettools
        ${CMAKE_SOURCE_DIR}/apps/gadgetron
        ${CMAKE_SOURCE_DIR}/apps/matlab
        ${CMAKE_SOURCE_DIR}/gadgets/mri_core 
        ${CMAKE_SOURCE_DIR}/gadgets/gtPlus
        ${ARMADILLO_INCLUDE_DIRS}
        )

add_library(gadgetron_dicom SHARED
    DicomFinishGadget.h DicomFinishGadget.cpp
    DicomImageWriter.h DicomImageWriter.cpp)

set_target_properties(gadgetron_dicom PROPERTIES VERSION ${GADGETRON_VERSION_STRING} SOVERSION ${GADGETRON_SOVERSION})

target_link_libraries(
    gadgetron_dicom
    gadgetron_gadgetbase
    gadgetron_toolbox_log
    gadgetron_toolbox_cpucore 
    ${ISMRMRD_LIBRARIES}
    optimized ${ACE_LIBRARIES}
    debug ${ACE_DEBUG_LIBRARY}
    ${Boost_LIBRARIES}
    ${GT_DICOM_LIBRARIES} )

if(ARMADILLO_FOUND)
    target_link_libraries(gadgetron_dicom gadgetron_toolbox_cpucore_math )
endif(ARMADILLO_FOUND)

if(MKL_FOUND)
    target_link_libraries(gadgetron_dicom gadgetron_toolbox_gtplus )
endif(MKL_FOUND)

install(
    FILES DicomFinishGadget.h DicomImageWriter.h gadgetron_dicom_export.h
    DESTINATION ${GADGETRON_INSTALL_INCLUDE_PATH} COMPONENT main)

install(TARGETS gadgetron_dicom DESTINATION lib COMPONENT main)

install(FILES dicom.xml DESTINATION ${GADGETRON_INSTALL_CONFIG_PATH} COMPONENT main)
