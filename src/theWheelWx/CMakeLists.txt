# CMakeLists.txt for theWheelWx - wxWidgets-based GUI application

# OpenGL ES renderer toggle
option(USE_OPENGL_RENDERER "Use OpenGL ES via ANGLE instead of 2D wxDC for rendering" OFF)

# When using vcpkg toolchain, wxWidgets may not be found via system paths.
# Hint the Homebrew location on macOS.
if(APPLE AND NOT wxWidgets_ROOT_DIR)
    execute_process(
        COMMAND wx-config --prefix
        OUTPUT_VARIABLE _wx_prefix
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(_wx_prefix)
        set(wxWidgets_ROOT_DIR "${_wx_prefix}")
        set(wxWidgets_CONFIG_EXECUTABLE "${_wx_prefix}/bin/wx-config")
    endif()
endif()

if(USE_OPENGL_RENDERER)
    find_package(wxWidgets COMPONENTS core base gl)
else()
    find_package(wxWidgets COMPONENTS core base)
endif()

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

if(USE_OPENGL_RENDERER)
    target_compile_definitions(theWheelWx PRIVATE USE_OPENGL_RENDERER)
    target_link_libraries(theWheelWx PRIVATE theWheelGL)
endif()

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
