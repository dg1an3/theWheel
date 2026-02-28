# CMakeLists.txt for theWheelWx GUI tests

# Reuse the Google Test fetched by theWheelModelTests (already available)
# If building standalone, fetch it here too
if(NOT TARGET GTest::gtest)
    include(FetchContent)
    FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

# Find wxWidgets (same logic as theWheelWx)
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

find_package(wxWidgets COMPONENTS core base)
if(NOT wxWidgets_FOUND)
    message(STATUS "wxWidgets not found - skipping theWheelWxTests")
    return()
endif()

include(${wxWidgets_USE_FILE})

# GUI source files from theWheelWx (including WheelApp.cpp for WheelFrame)
set(WX_GUI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../theWheelWx/WheelApp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../theWheelWx/SpacePanel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../theWheelWx/SpaceTreeView.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../theWheelWx/PropertyDialogs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../theWheelWx/SpaceSerializer.cpp
)

# Test sources
set(TEST_SOURCES
    test_wx_main.cpp
    test_space_panel.cpp
    test_space_tree_view.cpp
    test_wheel_frame.cpp
)

add_executable(theWheelWxTests ${TEST_SOURCES} ${WX_GUI_SOURCES})

target_compile_features(theWheelWxTests PRIVATE cxx_std_17)

target_link_libraries(theWheelWxTests
    PRIVATE
        theWheelModel
        OptimizeN
        ${wxWidgets_LIBRARIES}
        GTest::gtest
)

# Suppress wxIMPLEMENT_APP(WheelApp) — we provide our own wxApp in test_wx_main.cpp
target_compile_definitions(theWheelWxTests PRIVATE THEWHEEL_WX_TESTING)

target_include_directories(theWheelWxTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../theWheelWx
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Force-include wxcompat.h (same as theWheelWx)
if(NOT MSVC)
    target_compile_options(theWheelWxTests PRIVATE
        -include "${CMAKE_CURRENT_SOURCE_DIR}/../theWheelWx/wxcompat.h"
        -Wall -Wno-unused-variable -Wno-sign-compare)
endif()

# Register with CTest
include(GoogleTest)
gtest_discover_tests(theWheelWxTests)
