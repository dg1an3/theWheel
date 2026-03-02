# CMakeLists.txt for theWheelGLTest — standalone ANGLE renderer test harness

add_executable(theWheelGLTest WIN32 main.cpp)

target_link_libraries(theWheelGLTest
    PRIVATE
        theWheelGL
        theWheelModel
        OptimizeN
)

target_compile_features(theWheelGLTest PRIVATE cxx_std_17)

# theWheelView include dirs needed by StdAfx.h → RadialShape.h chain
target_include_directories(theWheelGLTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../theWheelView/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../theWheelView
)

# Force-include theWheelGL's StdAfx.h shim for REAL, CVectorD, etc.
target_compile_options(theWheelGLTest PRIVATE
    /FI"${CMAKE_CURRENT_SOURCE_DIR}/../theWheelGL/StdAfx.h"
)
