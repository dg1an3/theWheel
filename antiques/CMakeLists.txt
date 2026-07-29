# theWheel - Semantic Network Visualization System
# Root CMakeLists.txt
# Original code: Copyright (C) 1999-2002 Derek Graham Lane
# CMake build system: 2025

cmake_minimum_required(VERSION 3.15)

project(theWheel
    VERSION 0.1.0
    DESCRIPTION "Semantic network visualization system with Python bindings"
    LANGUAGES CXX C
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(BUILD_PYTHON_BINDINGS "Build Python bindings using pybind11" ON)
option(USE_INTEL_IPP "Use Intel IPP for optimizations in MTL" OFF)
option(BUILD_TESTS "Build test projects" OFF)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Build Python bindings: ${BUILD_PYTHON_BINDINGS}")

# Windows/MSVC specific settings
if(MSVC)
    # Use multi-threaded DLL runtime
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

    # Add compiler flags
    add_compile_options(/W3 /MP)

    # Disable specific warnings for old code
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    add_compile_definitions(_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
endif()

# Add subdirectories in dependency order
message(STATUS "Configuring XMLLogging...")
add_subdirectory(XMLLogging)

message(STATUS "Configuring MTL (Math Template Library)...")
add_subdirectory(MTL)

message(STATUS "Configuring OPTIMIZER_BASE...")
add_subdirectory(OptimizeN)

message(STATUS "Configuring THEWHEEL_MODEL...")
if(BUILD_TESTS)
    enable_testing()
endif()
add_subdirectory(THEWHEEL_MODEL)

# Build Python bindings if requested
if(BUILD_PYTHON_BINDINGS)
    message(STATUS "Configuring Python bindings...")
    add_subdirectory(python_bindings)
endif()

# Print summary
message(STATUS "")
message(STATUS "=== theWheel Build Configuration ===")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Python bindings: ${BUILD_PYTHON_BINDINGS}")
message(STATUS "  Intel IPP: ${USE_INTEL_IPP}")
message(STATUS "  Build tests: ${BUILD_TESTS}")
message(STATUS "====================================")
message(STATUS "")
