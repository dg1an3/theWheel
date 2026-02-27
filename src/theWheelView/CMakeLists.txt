# CMakeLists.txt for theWheelView library (DirectX 9, MSVC-only)

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
    Tracker.cpp
    stdafx.cpp
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

# Resource file
set(THEWHEELVIEW_RESOURCES
    theWheelView.rc
)

# Create static library
add_library(theWheelView STATIC
    ${THEWHEELVIEW_SOURCES}
    ${THEWHEELVIEW_HEADERS}
    ${THEWHEELVIEW_RESOURCES}
)

# Include directories
target_include_directories(theWheelView
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link with theWheelModel (brings OptimizeN transitively)
target_link_libraries(theWheelView
    PUBLIC
        theWheelModel
)

# Use MFC in a shared DLL
set_target_properties(theWheelView PROPERTIES
    COMPILE_DEFINITIONS "_AFXDLL"
)

# Precompiled headers
target_precompile_headers(theWheelView PRIVATE stdafx.h)
