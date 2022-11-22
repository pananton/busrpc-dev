#----------------------------------------------------------------------------------------------------------------------
# testing framework
#----------------------------------------------------------------------------------------------------------------------

include(FetchContent)
FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.tar.gz)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE) # do not override parent project's runtime settings on Windows
set(INSTALL_GTEST OFF)

# For simplicity, always build googletest as static library. This prevents mylib-tests executable from
# complaining about missing googletest DLLs on Windows.
set(BUILD_SHARED_LIBS OFF)

FetchContent_MakeAvailable(googletest)

#----------------------------------------------------------------------------------------------------------------------
# tests sources
#----------------------------------------------------------------------------------------------------------------------

set(sources
    command_tests.cpp
    check_command_tests.cpp
    configure_command_tests.cpp
    gendoc_command_tests.cpp
    help_command_tests.cpp
    imports_command_tests.cpp
    utils.h
    utils.cpp
    version_command_tests.cpp)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})

#----------------------------------------------------------------------------------------------------------------------
# tests target
#----------------------------------------------------------------------------------------------------------------------

add_executable(busrpc-tests)
target_sources(busrpc-tests PRIVATE ${sources})

target_include_directories(busrpc-tests
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(busrpc-tests
    PRIVATE
        busrpc-obj
        gtest_main)

include(GoogleTest)
gtest_discover_tests(busrpc-tests)
