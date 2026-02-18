# CMakeLists.txt for pythewheel - Python bindings for theWheel

# Python is already found in parent CMakeLists.txt
# Just create the Python module
# Using simplified bindings for now to avoid macro expansion issues
pybind11_add_module(pythewheel bindings_simple.cpp)

# Include directories for headers
target_include_directories(pythewheel PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../OptimizeN/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../theWheelModel/include
)

# Link with theWheelModel
target_link_libraries(pythewheel PRIVATE theWheelModel)

# Set C++ standard
target_compile_features(pythewheel PRIVATE cxx_std_17)

# Install to a known location for testing
# This will put the module in the build directory's python subfolder
set_target_properties(pythewheel PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/python"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/python"
)

# MSVC runtime is set globally to /MD (MultiThreadedDLL) in the root
# CMakeLists.txt via CMAKE_MSVC_RUNTIME_LIBRARY. No per-target CRT
# flags needed here.

# On MSVC, MFC flags are needed
if(MSVC)
    target_compile_definitions(pythewheel PRIVATE _AFXDLL NOMINMAX)
else()
    target_compile_definitions(pythewheel PRIVATE NOMINMAX)
    target_compile_options(pythewheel PRIVATE -Wall -Wno-unused-variable -Wno-sign-compare)
endif()
