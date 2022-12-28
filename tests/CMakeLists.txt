#----------------------------------------------------------------------------------------------------------------------
# testing framework
#----------------------------------------------------------------------------------------------------------------------

if(NOT DEFINED BUSRPC_GTEST_VERSION)
    set(BUSRPC_GTEST_VERSION "1.12.1"
        CACHE STRING "Version of GTest testing framework to be fetched" FORCE)
endif()

include("${PROJECT_SOURCE_DIR}/cmake/FetchGtest.cmake")
fetch_gtest(${BUSRPC_GTEST_VERSION})

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
    project_check_tests.cpp
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
