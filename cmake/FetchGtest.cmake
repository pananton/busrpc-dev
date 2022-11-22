# Provides function to fetch GTest testing framework and add it to the project.

include(FetchContent)

function(fetch_gtest tag)
    message(STATUS "Fetching GTest...")
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest
        GIT_TAG ${tag})

    set(BUILD_SHARED_LIBS OFF)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE INTERNAL "")

    FetchContent_MakeAvailable(googletest)
    message(STATUS "GTest added")
endfunction()
