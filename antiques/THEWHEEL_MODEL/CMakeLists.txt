# THEWHEEL_MODEL - Core semantic network model
cmake_minimum_required(VERSION 3.15)

project(THEWHEEL_MODEL)

# Source files (based on vcproj)
set(SOURCES
    Dib.cpp
    Node.cpp
    NodeLink.cpp
    Observer.cpp
    Space.cpp
    SpaceLayoutManager.cpp
    SpaceStateVector.cpp
    WAVE.C
    StdAfx.cpp
)

# Header files
set(HEADERS
    include/Attributes.h
    include/Dib.h
    include/Node.h
    include/NodeLink.h
    include/Observer.h
    include/Space.h
    include/SpaceLayoutManager.h
    include/SpaceStateVector.h
    include/WAVE.H
    StdAfx.h
)

# Create static library
add_library(THEWHEEL_MODEL STATIC ${SOURCES} ${HEADERS})

# Set include directories
target_include_directories(THEWHEEL_MODEL
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link dependencies
target_link_libraries(THEWHEEL_MODEL
    PUBLIC
        OPTIMIZER_BASE
        MTL
        XMLLogging
)

# Compiler definitions
target_compile_definitions(THEWHEEL_MODEL
    PRIVATE
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
        WIN32
        _WINDOWS
)

# Use precompiled headers (except for WAVE.C which is excluded)
target_precompile_headers(THEWHEEL_MODEL PRIVATE StdAfx.h)
set_source_files_properties(WAVE.C PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

# Set C++ standard
target_compile_features(THEWHEEL_MODEL PUBLIC cxx_std_11)

# Windows-specific settings
if(MSVC)
    target_compile_options(THEWHEEL_MODEL PRIVATE /W3)
    target_compile_definitions(THEWHEEL_MODEL PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _AFXDLL
        WINVER=0x0501
        _WIN32_WINNT=0x0501
    )
    # Set to use MFC in a shared DLL
    set_target_properties(THEWHEEL_MODEL PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
endif()

# DirectSound is also used - on Windows this is system library
if(WIN32)
    target_link_libraries(THEWHEEL_MODEL PRIVATE dsound dxguid)
endif()
