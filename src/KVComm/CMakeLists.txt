cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 14)
project(KVComm)

set(KVCOMM_MAJOR_VERSION 0)
set(KVCOMM_MINOR_VERSION 9)
set(KVCOMM_PATCH_VERSION 0)
set(KVCOMM_VERSION
    ${KVCOMM_MAJOR_VERSION}.${KVCOMM_MINOR_VERSION}.${KVCOMM_PATCH_VERSION})

add_library(kvcomm
    src/KV_Iterator.cpp
    src/KV_Builder.cpp
    src/KV_Parser.cpp
)
target_include_directories(kvcomm 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

set(HEADERS
    "include/KVComm/KV_Builder.hpp"
    "include/KVComm/KV_Builder.ipp"
    "include/KVComm/KV_Error.hpp"
    "include/KVComm/KV_Helpers.hpp"
    "include/KVComm/KV_Iterator.hpp"
    "include/KVComm/KV_Parser.hpp"
    "include/KVComm/KV_Types.hpp"
)
set_target_properties(kvcomm PROPERTIES
    PUBLIC_HEADER "${HEADERS}"
)

set(INSTALL_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/KVComm CACHE PATH
    "Installation directory for CMake files")

install(TARGETS kvcomm
    # Add the kvcomm library to the "export-set"
    EXPORT KVCommTargets
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT lib
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/KVComm" COMPONENT dev)

# Add all targets to the build-tree export set
export(TARGETS kvcomm
       FILE "${PROJECT_BINARY_DIR}/KVCommTargets.cmake")

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE kvcomm)

configure_file(FooBarConfigVersion.cmake.in
    "${PROJECT_BINARY_DIR}/FooBarConfigVersion.cmake" @ONLY)

# Install the KVCommConfig.cmake and KVCommConfigVersion.cmake
install(FILES
    "${PROJECT_SOURCE_DIR}${CMAKE_FILES_DIRECTORY}/KVCommConfig.cmake"
    "${PROJECT_BINARY_DIR}/FooBarConfigVersion.cmake"
    DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

# Install the export set for use with the install-tree
install(EXPORT KVCommTargets DESTINATION
    "${INSTALL_CMAKE_DIR}" COMPONENT dev)

################################################################################

add_library(kvcomm_arduino EXCLUDE_FROM_ALL
    src/KV_Iterator.cpp
    src/KV_Builder.cpp
    src/KV_Parser.cpp
)
target_include_directories(kvcomm_arduino 
    PUBLIC
        include
    PRIVATE
        src
)
target_compile_definitions(kvcomm_arduino PUBLIC ARDUINO_TEST)
target_link_libraries(kvcomm_arduino PRIVATE Arduino_Helpers)
