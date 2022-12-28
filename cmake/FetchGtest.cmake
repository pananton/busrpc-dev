# Provides function to fetch GTest testing framework and add it to the project.

include(FetchContent)

function(fetch_gtest version)
    message(STATUS "Fetching GTest ${version}...")
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-${version}.tar.gz)

    set(BUILD_SHARED_LIBS OFF)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE INTERNAL "")

    FetchContent_MakeAvailable(googletest)
    message(STATUS "GTest added")
endfunction()
