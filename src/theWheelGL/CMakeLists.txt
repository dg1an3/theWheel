# CMakeLists.txt for theWheelGL — cross-platform OpenGL ES renderer via ANGLE

# Source files
set(THEWHEELGL_SOURCES
    GLRenderer.cpp
    GLPlaque.cpp
    GLNodeViewSkin.cpp
    GLNodeView.cpp
    # Geometry source files wrapped with our own StdAfx.h shim
    GeometryWrappers.cpp
)

# Header files
set(THEWHEELGL_HEADERS
    include/GLRenderer.h
    include/GLPlaque.h
    include/GLNodeViewSkin.h
    include/GLNodeView.h
    include/GLMath.h
    Shaders.h
)

# Create static library
add_library(theWheelGL STATIC ${THEWHEELGL_SOURCES} ${THEWHEELGL_HEADERS})

# Include directories
target_include_directories(theWheelGL
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Depend on theWheelModel and OptimizeN for CNode, CExtent, CVectorD, etc.
target_link_libraries(theWheelGL
    PUBLIC
        theWheelModel
        OptimizeN
)

# ANGLE via vcpkg
find_package(unofficial-angle CONFIG)
if(unofficial-angle_FOUND)
    target_link_libraries(theWheelGL
        PUBLIC
            unofficial::angle::libEGL
            unofficial::angle::libGLESv2
    )
    # ANGLE's CMake targets may not export INTERFACE_INCLUDE_DIRECTORIES;
    # add the vcpkg installed headers path explicitly.
    if(VCPKG_INSTALLED_DIR)
        target_include_directories(theWheelGL PUBLIC
            "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include")
    endif()
    message(STATUS "theWheelGL: using ANGLE from vcpkg")
else()
    # Fallback: try to find system EGL/GLES2 (e.g., on Linux/macOS with native GL)
    find_package(OpenGL)
    if(OpenGL_EGL_FOUND AND OpenGL_GLESv2_FOUND)
        target_link_libraries(theWheelGL PUBLIC OpenGL::EGL OpenGL::GLESv2)
        message(STATUS "theWheelGL: using system EGL/GLESv2")
    else()
        message(WARNING "theWheelGL: neither ANGLE nor system EGL/GLESv2 found — "
                        "install ANGLE via vcpkg or provide EGL/GLESv2")
    endif()
endif()

# theWheelGL needs RadialShape/Elliptangle headers from theWheelView.
# Rather than moving those files now, just add theWheelView's include dir.
# On non-MSVC builds (macOS) theWheelView doesn't exist, so use the source path directly.
target_include_directories(theWheelGL
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../theWheelView/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../theWheelView
)

# Set C++ standard
target_compile_features(theWheelGL PRIVATE cxx_std_17)

# Force-include StdAfx.h shim to provide REAL, CVectorD, CExtent, ASSERT, RECT
# for all source files (needed by RadialShape.h and other theWheelView headers).
if(MSVC)
    target_compile_options(theWheelGL PRIVATE /FI"${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.h")
else()
    target_compile_options(theWheelGL PRIVATE
        -include "${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.h")
    # The wxcompat.h provides mfc_compat stubs needed by theWheelModel headers
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../theWheelWx/wxcompat.h")
        target_compile_options(theWheelGL PRIVATE
            -include "${CMAKE_CURRENT_SOURCE_DIR}/../theWheelWx/wxcompat.h")
    endif()
endif()
