cmake_minimum_required(VERSION 3.14)
project(busrpc-dev
    VERSION 1.0.0
    DESCRIPTION "Development tool for busrpc framework"
    LANGUAGES CXX)

#----------------------------------------------------------------------------------------------------------------------
# general settings and options
#----------------------------------------------------------------------------------------------------------------------

option(BUSRPC_BUILD_TESTS "Build busrpc development tool tests" OFF)

if(NOT DEFINED CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

#----------------------------------------------------------------------------------------------------------------------
# dependencies
#----------------------------------------------------------------------------------------------------------------------

find_package(Protobuf REQUIRED)

#----------------------------------------------------------------------------------------------------------------------
# sources
#----------------------------------------------------------------------------------------------------------------------

set(sources
    src/exception.h
    src/commands/command_args.h
    src/commands/command_args.cpp)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" FILES ${sources} src/main.cpp)

#----------------------------------------------------------------------------------------------------------------------
# target
#----------------------------------------------------------------------------------------------------------------------

add_library(busrpc-obj OBJECT ${sources})
add_executable(busrpc src/main.cpp)

target_include_directories(busrpc-obj PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(busrpc-obj PUBLIC protobuf::libprotobuf)
target_compile_features(busrpc-obj PUBLIC cxx_std_17)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" AND ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS "9.1")
    target_link_libraries(busrpc-obj PRIVATE stdc++fs)
endif()

target_link_libraries(busrpc PRIVATE busrpc-obj)

#----------------------------------------------------------------------------------------------------------------------
# other targets
#----------------------------------------------------------------------------------------------------------------------

if(BUSRPC_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
