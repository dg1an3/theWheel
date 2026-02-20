# CMakeLists.txt for OptimizeN library

# Collect all source files
set(OPTIMIZEN_SOURCES
    BrentOptimizer.cpp
    ConjGradOptimizer.cpp
    GradDescOptimizer.cpp
    LineFunction.cpp
    ObjectiveFunction.cpp
    Optimizer.cpp
    PowellOptimizer.cpp
)

# stdafx.cpp is only needed for MSVC precompiled headers
if(MSVC)
    list(APPEND OPTIMIZEN_SOURCES stdafx.cpp)
endif()

# Collect all header files
set(OPTIMIZEN_HEADERS
    include/BrentOptimizer.h
    include/ConjGradOptimizer.h
    include/GradDescOptimizer.h
    include/LineFunction.h
    include/MatrixNxM.h
    include/MatrixOps.h
    include/MathUtil.h
    include/ObjectiveFunction.h
    include/Optimizer.h
    include/PowellOptimizer.h
    include/Pseudoinverse.h
    include/UtilMacros.h
    include/VectorN.h
    include/VectorOps.h
    include/mfc_compat.h
    stdafx.h
)

# Create static library
add_library(OptimizeN STATIC ${OPTIMIZEN_SOURCES} ${OPTIMIZEN_HEADERS})

# Set C++ standard
target_compile_features(OptimizeN PUBLIC cxx_std_17)

# Include directories
target_include_directories(OptimizeN
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# MFC is required for this library on MSVC
if(MSVC)
    # Use MFC in a shared DLL
    set_target_properties(OptimizeN PROPERTIES
        COMPILE_DEFINITIONS "_AFXDLL"
    )

    # Precompiled headers
    target_precompile_headers(OptimizeN PRIVATE stdafx.h)
else()
    # GCC/Clang warning flags
    target_compile_options(OptimizeN PRIVATE -Wall -Wno-unused-variable -Wno-sign-compare)
endif()
