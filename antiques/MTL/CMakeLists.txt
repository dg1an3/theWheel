# MTL - Math Template Library with Intel IPP optimizations
cmake_minimum_required(VERSION 3.15)

project(MTL)

# Header-only library with some implementation files
set(HEADERS
    CastVectorD.h
    ComVariantMatrix.h
    GenUtils.h
    MathUtil.h
    MatrixD.h
    MatrixNxM.h
    MatrixNxM_pinv.h
    MatrixOps.h
    VectorD.h
    VectorN.h
    VectorOps.h
)

# Create interface library (header-only)
add_library(MTL INTERFACE)

# Set include directories
target_include_directories(MTL
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compiler definitions
# Note: USE_IPP is only defined if USE_INTEL_IPP option is enabled
# target_compile_definitions(MTL INTERFACE)

# Set C++ standard
target_compile_features(MTL INTERFACE cxx_std_11)

# Optional: Find and link Intel IPP if available
option(USE_INTEL_IPP "Use Intel IPP for optimizations" OFF)
if(USE_INTEL_IPP)
    # Try to find Intel IPP
    # Users would need to set IPP_ROOT or adjust paths
    find_path(IPP_INCLUDE_DIR ipp.h
        PATHS
            $ENV{IPPROOT}/include
            "C:/Program Files (x86)/Intel/oneAPI/ipp/latest/include"
    )

    if(IPP_INCLUDE_DIR)
        target_include_directories(MTL INTERFACE ${IPP_INCLUDE_DIR})
        target_compile_definitions(MTL INTERFACE USE_IPP)
        message(STATUS "Intel IPP found: ${IPP_INCLUDE_DIR}")
    else()
        message(WARNING "Intel IPP requested but not found")
    endif()
endif()
