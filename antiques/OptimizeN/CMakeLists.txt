# OPTIMIZER_BASE - Optimization algorithms framework
cmake_minimum_required(VERSION 3.15)

project(OPTIMIZER_BASE)

# Source files
set(SOURCES
    BrentOptimizer.cpp
    ConjGradOptimizer.cpp
    CubicInterpOptimizer.cpp
    DFPOptimizer.cpp
    GradDescOptimizer.cpp
    LineFunction.cpp
    ObjectiveFunction.cpp
    Optimizer.cpp
    PowellOptimizer.cpp
    StdAfx.cpp
)

# Header files
set(HEADERS
    include/BrentOptimizer.h
    include/ConjGradOptimizer.h
    include/CubicInterpOptimizer.h
    include/DFPOptimizer.h
    include/GradDescOptimizer.h
    include/LineFunction.h
    include/ObjectiveFunction.h
    include/Optimizer.h
    include/PowellOptimizer.h
    StdAfx.h
)

# Create static library
add_library(OPTIMIZER_BASE STATIC ${SOURCES} ${HEADERS})

# Set include directories
target_include_directories(OPTIMIZER_BASE
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link dependencies
target_link_libraries(OPTIMIZER_BASE
    PUBLIC
        MTL
        XMLLogging
)

# Compiler definitions
target_compile_definitions(OPTIMIZER_BASE
    PRIVATE
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
)

# Use precompiled headers
target_precompile_headers(OPTIMIZER_BASE PRIVATE StdAfx.h)

# Set C++ standard
target_compile_features(OPTIMIZER_BASE PUBLIC cxx_std_11)

# Windows-specific settings
if(MSVC)
    target_compile_options(OPTIMIZER_BASE PRIVATE /W3)
    target_compile_definitions(OPTIMIZER_BASE PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _AFXDLL
        WINVER=0x0501
        _WIN32_WINNT=0x0501
    )
    # Set to use MFC in a shared DLL
    set_target_properties(OPTIMIZER_BASE PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
endif()
