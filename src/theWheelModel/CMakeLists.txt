# CMakeLists.txt for theWheelModel library

# Collect all source files
set(THEWHEELMODEL_SOURCES
    Node.cpp
    NodeLink.cpp
    Space.cpp
    SpaceLayoutManager.cpp
    SpaceStateVector.cpp
)

# stdafx.cpp is only needed for MSVC precompiled headers
# Dib.cpp is Windows-only (uses MFC CBitmap, GDI, Video for Windows)
if(MSVC)
    list(APPEND THEWHEELMODEL_SOURCES stdafx.cpp Dib.cpp)
endif()

# Collect all header files
set(THEWHEELMODEL_HEADERS
    include/CastVectorD.h
    include/MatrixD.h
    include/Node.h
    include/NodeLink.h
    include/Space.h
    include/SpaceLayoutManager.h
    include/SpaceStateVector.h
    include/UpdateStamp.h
    include/VectorD.h
    stdafx.h
)

# Dib.h is Windows-only
if(MSVC)
    list(APPEND THEWHEELMODEL_HEADERS include/Dib.h)
endif()

# Create static library
add_library(theWheelModel STATIC ${THEWHEELMODEL_SOURCES} ${THEWHEELMODEL_HEADERS})

# Set C++ standard
target_compile_features(theWheelModel PUBLIC cxx_std_17)

# Include directories
target_include_directories(theWheelModel
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link with OptimizeN
target_link_libraries(theWheelModel
    PUBLIC
        OptimizeN
)

# MFC is required for this library on MSVC
if(MSVC)
    # Use MFC in a shared DLL
    set_target_properties(theWheelModel PROPERTIES
        COMPILE_DEFINITIONS "_AFXDLL"
    )

    # Precompiled headers
    target_precompile_headers(theWheelModel PRIVATE stdafx.h)
else()
    # GCC/Clang warning flags
    target_compile_options(theWheelModel PRIVATE -Wall -Wno-unused-variable -Wno-sign-compare)
endif()
