# Provides function to fetch nlohmann-json library and add it to the project.

include(FetchContent)

function(fetch_nlohmann_json version)
  message(STATUS "Fetching nlohmann-json library ${version}...")
  FetchContent_Declare(
      nlohmann_json
      URL https://github.com/nlohmann/json/archive/refs/tags/v${version}.tar.gz)

  set(JSON_BuildTests OFF CACHE INTERNAL "")
  FetchContent_MakeAvailable(nlohmann_json)
  message(STATUS "Nlohmann-json library added")
endfunction()
