# Provides function to fetch CLI11 library and add it to the project.

include(FetchContent)

function(fetch_cli11 tag)
  message(STATUS "Fetching CLI11 library")
  FetchContent_Declare(
      cli11
      GIT_REPOSITORY https://github.com/CLIUtils/CLI11
      GIT_TAG        ${tag})

  FetchContent_MakeAvailable(cli11)
  message(STATUS "CLI11 library added")
endfunction()
