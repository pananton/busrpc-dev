# Provides function to fetch CLI11 library and add it to the project.

include(FetchContent)

function(fetch_cli11 version)
  message(STATUS "Fetching CLI11 library ${version}...")
  FetchContent_Declare(
      cli11
      URL https://github.com/CLIUtils/CLI11/archive/refs/tags/v${version}.tar.gz)

  FetchContent_MakeAvailable(cli11)
  message(STATUS "CLI11 library added")
endfunction()
