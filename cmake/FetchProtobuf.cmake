# Provides function to fetch protobuf library and add it to the project.

include(FetchContent)

function(fetch_protobuf version)
    message(STATUS "Fetching Protobuf library ${version}...")
    FetchContent_Declare(
        protobuf
        URL https://github.com/protocolbuffers/protobuf/archive/refs/tags/v${version}.tar.gz)

    if(BUSRPC_USE_STATIC_PROTOBUF)
        set(BUILD_SHARED_LIBS OFF)
    else()
        set(BUILD_SHARED_LIBS ON)
    endif()

    set(protobuf_BUILD_TESTS OFF CACHE INTERNAL "")
    set(protobuf_BUILD_PROTOC_BINARIES OFF CACHE INTERNAL "")
    set(protobuf_INSTALL OFF CACHE INTERNAL "")

    FetchContent_MakeAvailable(protobuf)
    message(STATUS "Protobuf library added")
endfunction()
