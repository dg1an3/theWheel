# CMakeLists.txt for theWheelWx - wxWidgets-based GUI application

find_package(wxWidgets COMPONENTS core base)

if(NOT wxWidgets_FOUND)
    message(STATUS "wxWidgets not found - skipping theWheelWx build")
    return()
endif()

include(${wxWidgets_USE_FILE})

set(THEWHEELWX_SOURCES
    WheelApp.cpp
    SpaceTreeView.cpp
    SpacePanel.cpp
    PropertyDialogs.cpp
    SpaceSerializer.cpp
)

set(THEWHEELWX_HEADERS
    WheelApp.h
    SpaceTreeView.h
    SpacePanel.h
    PropertyDialogs.h
    SpaceSerializer.h
    Spring.h
)

add_executable(theWheelWx ${THEWHEELWX_SOURCES} ${THEWHEELWX_HEADERS})

target_compile_features(theWheelWx PRIVATE cxx_std_17)

target_link_libraries(theWheelWx
    PRIVATE
        theWheelModel
        OptimizeN
        ${wxWidgets_LIBRARIES}
)

target_include_directories(theWheelWx
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Force-include wxcompat.h to provide mfc_compat types before model headers
if(NOT MSVC)
    target_compile_options(theWheelWx PRIVATE
        -include "${CMAKE_CURRENT_SOURCE_DIR}/wxcompat.h"
        -Wall -Wno-unused-variable -Wno-sign-compare)
endif()

# macOS bundle settings
if(APPLE)
    set_target_properties(theWheelWx PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.thewheel.wx"
        MACOSX_BUNDLE_BUNDLE_NAME "theWheel"
    )
endif()
