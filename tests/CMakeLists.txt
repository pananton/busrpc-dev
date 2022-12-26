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

configure_file(tests_configure.h.in "${CMAKE_CURRENT_BINARY_DIR}/tests_configure.h")

set(sources
    bitwise_enum_tests.cpp
    utils_tests.cpp
    exception_tests.cpp
    error_collector_tests.cpp
    entity_docs_tests.cpp
    common_entity_tests.cpp
    project_entity_tests.cpp
    api_entity_tests.cpp
    namespace_entity_tests.cpp
    class_entity_tests.cpp
    method_entity_tests.cpp
    enum_entity_tests.cpp
    struct_entity_tests.cpp
    services_entity_tests.cpp
    service_entity_tests.cpp
    project_validation_tests.cpp
    parser_tests.cpp
    command_tests.cpp
    check_command_tests.cpp
    gendoc_command_tests.cpp
    help_command_tests.cpp
    imports_command_tests.cpp
    version_command_tests.cpp
    utils/common.h
    utils/common.cpp
    utils/file_utils.h
    utils/file_utils.cpp
    utils/project_utils.h
    utils/project_utils.cpp)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})

#----------------------------------------------------------------------------------------------------------------------
# tests target
#----------------------------------------------------------------------------------------------------------------------

add_executable(busrpc-tests)
target_sources(busrpc-tests PRIVATE ${sources})

target_include_directories(busrpc-tests
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_BINARY_DIR}")

target_link_libraries(busrpc-tests
    PRIVATE
        busrpc-obj
        gtest_main)

include(GoogleTest)
gtest_discover_tests(busrpc-tests)
