# XMLLogging - XML-based logging library
cmake_minimum_required(VERSION 3.15)

project(XMLLogging)

# Source files
set(SOURCES
    XMLElement.cpp
    XMLLoggableObject.cpp
    XMLLogFile.cpp
    StdAfx.cpp
)

# Header files
set(HEADERS
    XMLElement.h
    XMLLoggableObject.h
    XMLLogFile.h
    XMLLogging.h
    StdAfx.h
)

# Create static library
add_library(XMLLogging STATIC ${SOURCES} ${HEADERS})

# Set include directories
target_include_directories(XMLLogging
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compiler definitions
target_compile_definitions(XMLLogging
    PUBLIC
        USE_XMLLOGGING
    PRIVATE
        $<$<CONFIG:Debug>:XMLLOGGING_ON>
)

# Use precompiled headers (optional, can help with build times)
target_precompile_headers(XMLLogging PRIVATE StdAfx.h)

# Set C++ standard
target_compile_features(XMLLogging PUBLIC cxx_std_11)

# Windows-specific settings
if(MSVC)
    target_compile_options(XMLLogging PRIVATE /W3)
    target_compile_definitions(XMLLogging PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _AFXDLL
        WINVER=0x0501
        _WIN32_WINNT=0x0501
    )
    # Set to use MFC in a shared DLL
    set_target_properties(XMLLogging PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
endif()
