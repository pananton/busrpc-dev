# Provides function to fetch protobuf library and add it to the project.

include(FetchContent)

function(fetch_protobuf tag)
    message(STATUS "Fetching Protobuf library...")
    FetchContent_Declare(
        protobuf
        GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
        GIT_TAG ${tag})

    set(BUILD_SHARED_LIBS OFF)
    set(protobuf_BUILD_TESTS OFF CACHE INTERNAL "")
    set(protobuf_BUILD_PROTOC_BINARIES OFF CACHE INTERNAL "")
    set(protobuf_INSTALL OFF CACHE INTERNAL "")

    FetchContent_MakeAvailable(protobuf)
    message(STATUS "Protobuf library added")
endfunction()

