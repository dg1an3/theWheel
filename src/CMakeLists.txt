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

project ("theWheel_src" LANGUAGES CXX)

# Use release DLL runtime (/MD) for all configurations, including Debug.
# Python extensions must match the Python interpreter's CRT, which is /MD.
# All static libraries in this tree must use the same CRT for consistent linking.
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build static libraries with -fPIC so they can be linked into the Python
# extension module (pythewheel) on Linux. No effect on MSVC.
if(NOT MSVC)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# Python bindings (pythewheel): available on Windows and on platforms where
# Python and pybind11 can be located. pybind11 is fetched via FetchContent.
find_package(Python3 COMPONENTS Interpreter Development QUIET)
if(Python3_FOUND)
    set(PYBIND11_FINDPYTHON ON)

    include(FetchContent)
    FetchContent_Declare(
      pybind11
      GIT_REPOSITORY https://github.com/pybind/pybind11.git
      GIT_TAG v2.13.6
    )
    FetchContent_MakeAvailable(pybind11)
    set(PYTHEWHEEL_AVAILABLE ON)
else()
    message(STATUS "Skipping pythewheel — Python3 development headers not found")
    set(PYTHEWHEEL_AVAILABLE OFF)
endif()

# Enable testing
enable_testing()

# Include sub-projects.
add_subdirectory ("OptimizeN")
add_subdirectory ("theWheelModel")

# Cross-platform OpenGL ES renderer (via ANGLE) — optional
find_package(unofficial-angle CONFIG QUIET)
if(unofficial-angle_FOUND OR (OpenGL_EGL_FOUND AND OpenGL_GLESv2_FOUND))
    add_subdirectory ("theWheelGL")
else()
    message(STATUS "Skipping theWheelGL — no ANGLE or EGL/GLESv2 available")
endif()

if(PYTHEWHEEL_AVAILABLE)
    add_subdirectory ("pybind")
endif()

if(MSVC)
    add_subdirectory ("theWheelView")
    add_subdirectory ("theWheel")
endif()

# ANGLE test harness (Windows only, requires theWheelGL)
if(MSVC AND TARGET theWheelGL)
    add_subdirectory ("theWheelGLTest")
endif()

add_subdirectory ("theWheelModelTests")

# wxWidgets application (macOS/Linux)
if(NOT MSVC)
    add_subdirectory ("theWheelWx")
    add_subdirectory ("theWheelWxTests")
endif()
