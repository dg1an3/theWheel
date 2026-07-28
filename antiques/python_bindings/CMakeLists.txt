# Python bindings for THEWHEEL_MODEL using pybind11
cmake_minimum_required(VERSION 3.15)

project(thewheel_python)

# Find Python
find_package(Python COMPONENTS Interpreter Development REQUIRED)

# Try to find pybind11
find_package(pybind11 CONFIG)

if(NOT pybind11_FOUND)
    # If pybind11 is not found, try to fetch it
    include(FetchContent)
    FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11
        GIT_TAG        v2.11.1
    )
    FetchContent_MakeAvailable(pybind11)
endif()

# Create the Python module
pybind11_add_module(thewheel thewheel_bindings.cpp)

# Link against THEWHEEL_MODEL and its dependencies
target_link_libraries(thewheel
    PRIVATE
        THEWHEEL_MODEL
        OPTIMIZER_BASE
        MTL
        XMLLogging
)

# On Windows, we need to handle MFC properly
if(WIN32 AND MSVC)
    # Need to use MFC since the libraries depend on it
    target_compile_definitions(thewheel PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _AFXDLL
        WIN32
        _WINDOWS
        WINVER=0x0501
        _WIN32_WINNT=0x0501
    )

    # Set to use MFC in a shared DLL (must match the libraries)
    set_target_properties(thewheel PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )

    # Link DirectSound and multimedia libraries which are used by THEWHEEL_MODEL
    target_link_libraries(thewheel PRIVATE dsound dxguid winmm)
endif()

# Set C++ standard
target_compile_features(thewheel PRIVATE cxx_std_11)

# Install the module
install(TARGETS thewheel
    LIBRARY DESTINATION .
    RUNTIME DESTINATION .
)
