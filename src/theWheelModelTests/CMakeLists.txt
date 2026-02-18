# CMakeLists.txt for theWheelModel unit tests

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)

# Required: project uses /MD for all configurations
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Collect test sources
set(TEST_SOURCES
    test_main.cpp
    test_vectord.cpp
    test_nodelink.cpp
    test_node.cpp
    test_space.cpp
    test_space_layout_manager.cpp
)

# Create test executable
add_executable(theWheelModelTests ${TEST_SOURCES})

# Link against theWheelModel and Google Test
target_link_libraries(theWheelModelTests
    PRIVATE
        theWheelModel
        GTest::gtest
)

# Include theWheelModel source dir for stdafx.h access
target_include_directories(theWheelModelTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../theWheelModel
)

# MSVC-specific: need _AFXDLL for MFC
if(MSVC)
    target_compile_definitions(theWheelModelTests PRIVATE _AFXDLL)
endif()

# Register tests with CTest
include(GoogleTest)
gtest_discover_tests(theWheelModelTests)
