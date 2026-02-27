# CMakeLists.txt for theWheelView library (Windows/DirectX only)

if(NOT MSVC)
    return()
endif()

# DirectX 9 headers and libraries ship with the Windows 10 SDK.
# The legacy June 2010 DirectX SDK is not required.

# Source files
set(THEWHEELVIEW_SOURCES
    DesigntimeTracker.cpp
    Elliptangle.cpp
    Molding.cpp
    NodeLayoutManager.cpp
    NodeView.cpp
    NodeViewSkin.cpp
    NodeViewSkin2007.cpp
    Plaque.cpp
    RadialShape.cpp
    RuntimeTracker.cpp
    SpaceView.cpp
    Spring.cpp
    stdafx.cpp
    Tracker.cpp
)

# Header files
set(THEWHEELVIEW_HEADERS
    include/DesigntimeTracker.h
    include/Elliptangle.h
    include/Extent.h
    include/Molding.h
    include/NodeLayoutManager.h
    include/NodeView.h
    include/NodeViewSkin.h
    include/NodeViewSkin2007.h
    include/Plaque.h
    include/RadialShape.h
    include/RuntimeTracker.h
    include/SpaceView.h
    include/Spring.h
    include/Tracker.h
    D3DXImpl.h
    THEWHEEL_VIEW_resource.h
    stdafx.h
)

# Create static library
add_library(theWheelView STATIC ${THEWHEELVIEW_SOURCES} ${THEWHEELVIEW_HEADERS})

# Include directories
target_include_directories(theWheelView
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link dependencies
target_link_libraries(theWheelView
    PUBLIC
        theWheelModel
        OptimizeN
    PRIVATE
        d3d9.lib
        dxguid.lib
        winmm.lib
)

# OpenGL ES renderer toggle: define USE_OPENGL_RENDERER to use ANGLE instead of D3D9
option(USE_OPENGL_RENDERER "Use OpenGL ES via ANGLE instead of D3D9 for rendering" OFF)
if(USE_OPENGL_RENDERER)
    target_compile_definitions(theWheelView PUBLIC USE_OPENGL_RENDERER)
    target_link_libraries(theWheelView PUBLIC theWheelGL)
endif()

# MFC in shared DLL
target_compile_definitions(theWheelView PRIVATE _AFXDLL)

# Precompiled headers
target_precompile_headers(theWheelView PRIVATE stdafx.h)
