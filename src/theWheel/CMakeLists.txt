# CMakeLists.txt for theWheel MFC application (Windows only)

if(NOT MSVC)
    return()
endif()

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

# MFC application (WIN32 = Windows subsystem, not console)
add_executable(theWheelApp WIN32
    ${THEWHEEL_SOURCES}
    ${THEWHEEL_HEADERS}
    ${THEWHEEL_RESOURCES}
)

# Use dynamic MFC
set(CMAKE_MFC_FLAG 2)
set_target_properties(theWheelApp PROPERTIES
    COMPILE_DEFINITIONS "_AFXDLL"
    OUTPUT_NAME "theWheel"
)

# Include directories
target_include_directories(theWheelApp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link dependencies
target_link_libraries(theWheelApp
    PRIVATE
        theWheelView
        theWheelModel
        OptimizeN
        d3d9.lib
        ddraw.lib
        dsound.lib
        dxguid.lib
        winmm.lib
)

# Precompiled headers
target_precompile_headers(theWheelApp PRIVATE stdafx.h)
