cmake_minimum_required(VERSION 3.14)
project(busrpc-dev
    VERSION 1.0.0
    DESCRIPTION "Development tool for busrpc framework"
    LANGUAGES CXX)

#----------------------------------------------------------------------------------------------------------------------
# general settings and options
#----------------------------------------------------------------------------------------------------------------------

option(BUSRPC_BUILD_TESTS "Build busrpc development tool tests" OFF)

if(NOT DEFINED BUSRPC_CLI11_VERSION)
    set(BUSRPC_CLI11_VERSION 2.3.1 CACHE STRING "Version of CLI11 library to be used by busrpc development tool" FORCE)
endif()

if(NOT DEFINED CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

#----------------------------------------------------------------------------------------------------------------------
# dependencies
#----------------------------------------------------------------------------------------------------------------------

include(FetchContent)

FetchContent_Declare(
    cli11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11
    GIT_TAG        v${BUSRPC_CLI11_VERSION})

FetchContent_MakeAvailable(cli11)

#----------------------------------------------------------------------------------------------------------------------
# sources
#----------------------------------------------------------------------------------------------------------------------

configure_file(src/configure.h.in "${CMAKE_CURRENT_BINARY_DIR}/src/configure.h")

set(sources
    src/app.h
    src/app.cpp
    src/exception.h
    src/commands/command.h
    src/commands/command.cpp
    src/commands/command_id.h
    src/commands/check/check_command.h
    src/commands/check/check_command.cpp
    src/commands/configure/configure_command.h
    src/commands/configure/configure_command.cpp
    src/commands/gendoc/gendoc_command.h
    src/commands/gendoc/gendoc_command.cpp
    src/commands/help/help_command.h
    src/commands/help/help_command.cpp
    src/commands/imports/imports_command.h
    src/commands/imports/imports_command.cpp
    src/commands/version/version_command.h
    src/commands/version/version_command.cpp)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" FILES ${sources} src/main.cpp)

#----------------------------------------------------------------------------------------------------------------------
# target
#----------------------------------------------------------------------------------------------------------------------

add_library(busrpc-obj OBJECT ${sources})
add_executable(busrpc src/main.cpp)

target_compile_features(busrpc-obj PUBLIC cxx_std_20)

target_include_directories(busrpc-obj
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_BINARY_DIR}/src")

target_link_libraries(busrpc-obj
    PUBLIC
        CLI11::CLI11)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" AND ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS "9.1")
    target_link_libraries(busrpc-obj PRIVATE stdc++fs)
    target_compile_definitions(busrpc-obj PUBLIC CLI11_HAS_FILESYSTEM=1)
endif()

target_link_libraries(busrpc PRIVATE busrpc-obj)

#----------------------------------------------------------------------------------------------------------------------
# other targets
#----------------------------------------------------------------------------------------------------------------------

if(BUSRPC_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
