cmake_minimum_required(VERSION 3.14)
project(busrpc-dev
    VERSION 1.0.0
    DESCRIPTION "Development tool for busrpc framework"
    LANGUAGES CXX)

#----------------------------------------------------------------------------------------------------------------------
# general settings and options
#----------------------------------------------------------------------------------------------------------------------

option(BUSRPC_BUILD_TESTS "Build busrpc development tool tests" OFF)
option(BUSRPC_USE_EXTERNAL_CLI11 "Use external CLI11 library" OFF)
option(BUSRPC_USE_EXTERNAL_PROTOBUF "User external protobuf library" OFF)

if(NOT BUSRPC_USE_EXTERNAL_CLI11 AND NOT DEFINED BUSRPC_CLI11_TAG)
    set(BUSRPC_CLI11_TAG c2ea58c7f9bb2a1da2d3d7f5b462121ac6a07f16 # v2.3.1
        CACHE STRING "Tag or commit hash of CLI11 library to be fetched" FORCE)
endif()

if(NOT BUSRPC_USE_EXTERNAL_PROTOBUF AND NOT DEFINED BUSRPC_PROTOBUF_TAG)
    set(BUSRPC_PROTOBUF_TAG 90b73ac3f0b10320315c2ca0d03a5a9b095d2f66 # v21.9
        CACHE STRING "Tag or commit hash of Protobuf library to be fetched" FORCE)
endif()

if(NOT DEFINED CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

#----------------------------------------------------------------------------------------------------------------------
# dependencies
#----------------------------------------------------------------------------------------------------------------------

if(NOT BUSRPC_USE_EXTERNAL_CLI11)
    include("cmake/FetchCLI11.cmake")
    fetch_cli11(${BUSRPC_CLI11_TAG})
else()
    find_package(CLI11 CONFIG REQUIRED)
endif()

if(NOT BUSRPC_USE_EXTERNAL_PROTOBUF)
    include("cmake/FetchProtobuf.cmake")
    fetch_protobuf(${BUSRPC_PROTOBUF_TAG})
    set(BUSRPC_TESTS_PROTOBUF_ROOT "${protobuf_SOURCE_DIR}/src")
else()
    find_package(Protobuf REQUIRED)
    set(BUSRPC_TESTS_PROTOBUF_ROOT "${Protobuf_INCLUDE_DIR}")
endif()

#----------------------------------------------------------------------------------------------------------------------
# sources
#----------------------------------------------------------------------------------------------------------------------

configure_file(src/configure.h.in "${CMAKE_CURRENT_BINARY_DIR}/src/configure.h")

set(sources
    src/app.h
    src/app.cpp
    src/constants.h
    src/error_collector.h
    src/error_collector.cpp
    src/exception.h
    src/protobuf_error_collector.h
    src/types.h
    src/utils.h
    src/utils.cpp
    src/commands/command.h
    src/commands/command.cpp
    src/commands/check/check_command.h
    src/commands/check/check_command.cpp
    src/commands/gendoc/gendoc_command.h
    src/commands/gendoc/gendoc_command.cpp
    src/commands/help/help_command.h
    src/commands/help/help_command.cpp
    src/commands/imports/imports_command.h
    src/commands/imports/imports_command.cpp
    src/commands/version/version_command.h
    src/commands/version/version_command.cpp
    src/entities/api.h
    src/entities/api.cpp
    src/entities/class.h
    src/entities/class.cpp
    src/entities/constant.h
    src/entities/constant.cpp
    src/entities/entity.cpp
    src/entities/entity.h
    src/entities/enum.h
    src/entities/enum.cpp
    src/entities/field.h
    src/entities/field.cpp
    src/entities/method.h
    src/entities/method.cpp
    src/entities/namespace.h
    src/entities/namespace.cpp
    src/entities/project.h
    src/entities/project.cpp
    src/entities/service.h
    src/entities/service.cpp
    src/entities/services.h
    src/entities/services.cpp
    src/entities/struct.h
    src/entities/struct.cpp
    src/parser/parser.h
    src/parser/parser.cpp)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" FILES ${sources} src/main.cpp)

#----------------------------------------------------------------------------------------------------------------------
# target
#----------------------------------------------------------------------------------------------------------------------

add_library(busrpc-obj OBJECT ${sources})
add_executable(busrpc src/main.cpp)

target_compile_features(busrpc-obj PUBLIC cxx_std_20)

separate_arguments(warnings NATIVE_COMMAND "${BUSRPC_WARNINGS}")
target_compile_options(busrpc-obj PUBLIC ${warnings})



target_include_directories(busrpc-obj
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_BINARY_DIR}/src")

target_link_libraries(busrpc-obj
    PUBLIC
        CLI11::CLI11
        protobuf::libprotobuf)

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
