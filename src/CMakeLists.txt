# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.10...3.31)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Enable per-target MSVC runtime library selection (CMake 3.15+).
if (POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif()

project ("theWheel_src")

# Use release DLL runtime (/MD) for all configurations, including Debug.
# Python extensions must match the Python interpreter's CRT, which is /MD.
# All static libraries in this tree must use the same CRT for consistent linking.
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Python before pybind11 to use modern CMake Python finding
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

# Configure pybind11 to use modern Python finding
set(PYBIND11_FINDPYTHON ON)

# Fetch pybind11 for Python bindings
include(FetchContent)
FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11.git
  GIT_TAG v2.13.6
)
FetchContent_MakeAvailable(pybind11)

# Enable testing
enable_testing()

# Include sub-projects.
add_subdirectory ("OptimizeN")
add_subdirectory ("theWheelModel")
add_subdirectory ("pythewheel")
add_subdirectory ("OptimizeND")
add_subdirectory ("theWheelModelTests")
