# busrpc-dev

Command-line development tool for a [busrpc](https://github.com/pananton/busrpc-spec) microservice developers.

# Building

To build busrpc development tool CMake version not older than 3.14 is required.

Busrpc development tool depends on the following libraries and tools:
* [CLI11](https://github.com/CLIUtils/CLI11) for command-line parsing
* [Protobuf](https://github.com/protocolbuffers/protobuf) for parsing protobuf files
* [nlohmann_json](https://github.com/nlohmann/json) for generating JSON documentation
* [Gtest](https://github.com/google/googletest) for unit testing

However, by default tool will download them and add to it's own build using CMake's [FetchContent] mechanism. That means, most of the times project will be configured for building with this simple instructions:

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=<TYPE> ..
```

For more granular control over the build process the following CMake variables are provided:
* `BUSRPC_BUILD_TESTS` (default `ON`) to enable/disable building of the project unit tests
* `BUSRPC_BUILD_DOCS` (default `OFF`) to enable/disable documentation generation from project sources (doxygen should be installed and available on a well-known path)
* `BUSRPC_CLI11_FETCH_VERSION`, `BUSRPC_PROTOBUF_FETCH_VERSION`, `BUSRPC_NLOHMANN_JSON_FETCH_VERSION`, `BUSRPC_GTEST_FETCH_VERSION` - for choosing which version of the dependency to fetch (should be only digits and dots, no leading 'v' should be specified)
* `BUSRPC_USE_EXTERNAL_CLI11`, `BUSRPC_USE_EXTERNAL_PROTOBUF`, `BUSRPC_USE_EXTERNAL_NLOHMANN_JSON` if you want to use externally installed dependencies instead of downloaded ones
* `BUSRPC_WARNINGS` and `BUSRPC_WARNINGS_AS_ERRORS` to control warning level of the build


