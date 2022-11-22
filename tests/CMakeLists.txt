#----------------------------------------------------------------------------------------------------------------------
# testing framework
#----------------------------------------------------------------------------------------------------------------------

if(NOT DEFINED BUSRPC_GTEST_TAG)
    set(BUSRPC_GTEST_TAG 58d77fa8070e8cec2dc1ed015d66b454c8d78850 # v1.12.1
        CACHE STRING "Tag or commit hash of GTest testing framework to be fetched" FORCE)
endif()

include("${PROJECT_SOURCE_DIR}/cmake/FetchGtest.cmake")
fetch_gtest(${BUSRPC_GTEST_TAG})

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
