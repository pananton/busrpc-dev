# busrpc-dev

Command-line development tool for a [busrpc](https://github.com/pananton/busrpc-spec) microservice developers.

# Building

To build busrpc development tool CMake version not older than 3.14 is required.

Busrpc development tool depends on the following libraries and tools:
* [CLI11](https://github.com/CLIUtils/CLI11) for command-line parsing
* [Protobuf](https://github.com/protocolbuffers/protobuf) for parsing protobuf files
* [nlohmann_json](https://github.com/nlohmann/json) for generating JSON documentation
* [Gtest](https://github.com/google/googletest) for unit testing

However, by default tool will download them and add to it's own build using CMake's [FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html) mechanism. That means, most of the time project can be configured for building with this simple instructions:

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=<TYPE> ..
```

For more granular control over the build process the following CMake variables are provided:
* `BUSRPC_BUILD_TESTS` (default `ON`) to enable/disable building of the project unit tests
* `BUSRPC_BUILD_DOCS` (default `OFF`) to enable/disable documentation generation from project sources (doxygen should be installed and available on a well-known path)
* `BUSRPC_CLI11_FETCH_VERSION`, `BUSRPC_PROTOBUF_FETCH_VERSION`, `BUSRPC_NLOHMANN_JSON_FETCH_VERSION`, `BUSRPC_GTEST_FETCH_VERSION` - for choosing which version of the dependency to fetch (should contain only digits and dots, no leading 'v' should be specified)
* `BUSRPC_USE_EXTERNAL_CLI11`, `BUSRPC_USE_EXTERNAL_PROTOBUF`, `BUSRPC_USE_EXTERNAL_NLOHMANN_JSON` if you want to use externally installed dependencies instead of downloaded one
* `BUSRPC_WARNINGS` and `BUSRPC_WARNINGS_AS_ERRORS` to control warning level of the build

Also if your CMake version is 3.21 or higher, CMake preset can be used for controlling project options. Two presets are provided in the *CMakePresets.json* file: `dev` for configuring build for development and `release` for building release version. To build with presets, execute:

```
cmake --preset dev|release
```

# Using docker image

Busrpc development tool is also available in the docker hub and can be run using this instruction:

```
docker run pananton/busrpc:latest <COMMAND>
```
 
# Commands

To list available busrpc development tool commands run `busprc` without arguments:

```
$ ./busrpc
Busrpc development tool
Usage: busrpc [OPTIONS] [SUBCOMMAND]

Options:
  -h,--help                   Print this help message and exit
  -v,--version                Display program version information and exit

Subcommands:
  check                       Check API for conformance to the busrpc specification
  gendoc                      Generate API documentation
  help                        Show help about the command
  imports                     Output relative paths to the files directly or indirectly imported by the specified file(s)
  version                     Show version information
```


## `check`

SYNOPSIS

```
busrpc check [-h] [-r PROJECT_DIR] [-p PROTOBUF_ROOT]
             [--ignore-spec] [--ignore-doc] [--ignore-style] [-w]
```

DESCRIPTION

Check busrpc project for compliance with the [specification](https://github.com/pananton/busrpc-spec).

OPTIONS

* `-h`, `--help` - print help message and exit
* `-r`, `--root` - busrpc project directory
* `-p`, `--protobuf-root` - root directory for built-in protobuf *proto* files
* `--ignore-spec` - ignore specification warnings
* `--ignore-doc` - ignore documentation warnings
* `--ignore-style` - ignore busrpc style warnings
* `-w`, `--warning-as-error` - treat warnings as errors

NOTES

Busrpc project directory is the directory which contains *busrpc.proto* file. If project directory parameter `-r` is not specified on the command line, development tool also looks for `BUSRPC_PROJECT_DIR` environment variable and uses it's value if variable exists. As a final attempt, development tool assumes working directory to be a project directory.

Every busrpc project imports at least *google/protobuf/descriptor.proto* file to define busrpc custom protobuf options (like `observable` and `hashed`), which means that development tool should be provided with information where to find this files.

If protobuf root parameter `-p` is not specified on the command line, development tool also looks for `BUSRPC_PROTOBUF_ROOT` environment variable and uses it's value if variable exists. Also on *NIX systems `/usr/include` and `/usr/local/include` are searched.

RESULT

Returns 0 if all checks have been passed, non-zero otherwise.


## `gendoc`

SYNOPSIS

```
busrpc gendoc [-h] [-r PROJECT_DIR] [-p PROTOBUF_ROOT] [-d OUTPUT_DIR]
              [--format FORMAT]
```

DESCRIPTION

Generate busrpc project documentation.

OPTIONS

* `-h`, `--help` - print help message and exit
* `-r`, `--root` - busrpc project directory
* `-p`, `--protobuf-root` - root directory for built-in protobuf *proto* files
* `-d`, `--output-dir` - directory where to write generated documentation (working directory is used by default)
* `--format` - documentation format (currently only `json` is supported, which is also the default value)

NOTES

For more information about `-r` and `-p` options see section NOTES of the [`check`](#check) command.

Information about format of the generated JSON documentation can be found [here](#json-documentation-schema).

If project has specification or other errors, then this command still tries to generate as much documentation as possible but reports error result (see below). In this case generated documentation may be inconsistent, but it still syntactically represents a valid JSON.

RESULT

Returns 0 if documentation is generatead, non-zero otherwise.


## `help`

SYNOPSIS

```
busrpc help [-h] [COMMAND]
```

DESCRIPTION

Prints help message about `COMMAND`. If `COMMAND` is not specified, outputs list of available commands.

OPTIONS

* `-h`, `--help` - print help message and exit


## `imports`

SYNOPSIS

```
busrpc imports [-h] [-r PROJECT_DIR] [-p PROTOBUF_ROOT] [--only-deps]
               [FILES]...
```

DESCRIPTION

Output relative paths to the files directly or indirectly imported by FILES.

OPTIONS

* `-h`, `--help` - print help message and exit
* `-r`, `--root` - busrpc project directory
* `-p`, `--protobuf-root` - root directory for built-in protobuf *proto* files
* `--only-deps` - only output paths to the dependencies, do not output paths to FILES

NOTES

For more information about `-r` and `-p` options see section NOTES of the [`check`](#check) command.


## `version`

SYNOPSIS

```
busrpc version [-h]
```

DESCRIPTION

Prints busrpc development tool version information.

OPTIONS

* `-h,--help` - print help message and exit

# JSON documentation schema
