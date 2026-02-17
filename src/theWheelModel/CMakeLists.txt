# CMakeLists.txt for theWheelModel library

# Collect all source files
set(THEWHEELMODEL_SOURCES
    Dib.cpp
    Node.cpp
    NodeLink.cpp
    Space.cpp
    SpaceLayoutManager.cpp
    SpaceStateVector.cpp
    stdafx.cpp
)

# Collect all header files
set(THEWHEELMODEL_HEADERS
    include/CastVectorD.h
    include/Dib.h
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

# MFC is required for this library
if(MSVC)
    # Use MFC in a shared DLL
    set_target_properties(theWheelModel PROPERTIES
        COMPILE_DEFINITIONS "_AFXDLL"
    )

    # Precompiled headers
    target_precompile_headers(theWheelModel PRIVATE stdafx.h)
endif()
