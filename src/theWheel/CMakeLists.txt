# CMakeLists.txt for theWheel MFC application (MSVC-only)

# Source files
set(THEWHEEL_SOURCES
    ImagePropDlg.cpp
    LeftView.cpp
    LinkPropDlg.cpp
    MainFrm.cpp
    NodePropDlg.cpp
    spaceclassdlg.cpp
    spaceoptdlg.cpp
    spacepropdlg.cpp
    SpaceTreeView.cpp
    stdafx.cpp
    theWheel.cpp
    theWheelDoc.cpp
    theWheelView.cpp
    TreeMenuCtrl.cpp
)

# Header files
set(THEWHEEL_HEADERS
    imagepropdlg.h
    LeftView.h
    linkpropdlg.h
    MainFrm.h
    NodePropDlg.h
    Resource.h
    spaceclassdlg.h
    spaceoptdlg.h
    spacepropdlg.h
    SpaceTreeView.h
    stdafx.h
    theWheel.h
    theWheelDoc.h
    theWheelView.h
    TreeMenuCtrl.h
)

# Resource file
set(THEWHEEL_RESOURCES
    theWheel.rc
)

# Create Win32 executable (not console)
add_executable(theWheel WIN32
    ${THEWHEEL_SOURCES}
    ${THEWHEEL_HEADERS}
    ${THEWHEEL_RESOURCES}
)

# Include directories
target_include_directories(theWheel
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link with theWheelView (brings theWheelModel + OptimizeN transitively)
# System libs for DirectX 9 and multimedia (available in Windows SDK)
target_link_libraries(theWheel
    PRIVATE
        theWheelView
        d3d9.lib
        ddraw.lib
        dxguid.lib
        dsound.lib
        winmm.lib
        urlmon.lib
        wininet.lib
        msacm32.lib
)

# Use MFC in a shared DLL
set_target_properties(theWheel PROPERTIES
    COMPILE_DEFINITIONS "_AFXDLL"
)

# Precompiled headers
target_precompile_headers(theWheel PRIVATE stdafx.h)
