#pragma once

#include "commands/command.h"

#include <filesystem>
#include <functional>
#include <string>
#include <system_error>

/// \dir commands/gendoc Types and utilites for \c gendoc command implementation.
/// \file command.h Command \c gendoc implementation.

namespace CLI {
class App;
}

namespace busrpc {

/// Command-specific error code.
enum class GenDocErrc {
    /// Busrpc specification violated.
    Spec_Violated = 1,

    /// Failed to parse protobuf file.
    Protobuf_Parsing_Failed = 2,

    /// Failed to read a source file for documentation.
    File_Read_Failed = 3,

    /// Failed to write generated documentation to the output directory.
    File_Write_Failed = 4,

    /// Busrpc project directory does not exist or does not represent a valid project directory.
    Invalid_Project_Dir = 5
};

/// Return error category for the \c gendoc command.
const std::error_category& gendoc_error_category();

/// Create error code from the \ref GenDocErrc value.
std::error_code make_error_code(GenDocErrc errc);

/// Format of the generated documentation.
enum class GenDocFormat {
    /// JSON.
    Json = 1
};

/// Return string representation of a documentation format.
/// \note \c nullptr is returned if \a lang is unknown.
constexpr const char* GetGenDocFormatStr(GenDocFormat lang)
{
    switch (lang) {
    case GenDocFormat::Json: return "json";
    default: return nullptr;
    }
}

/// Arguments of the \c gendoc command.
class GenDocArgs {
public:
    /// Create \c gendoc command arguments.
    GenDocArgs(GenDocFormat format = GenDocFormat::Json,
               std::filesystem::path projectDir = std::filesystem::current_path(),
               std::filesystem::path outputDir = std::filesystem::current_path(),
               std::filesystem::path protobufRootDir = {}):
        format_(format),
        projectDir_(std::move(projectDir)),
        outputDir_(std::move(outputDir)),
        protobufRootDir_(std::move(protobufRootDir))
    { }

    /// Format of the documentation.
    GenDocFormat format() const noexcept { return format_; }

    /// Busrpc project directory.
    /// \note If empty, working directory is assumed.
    const std::filesystem::path& projectDir() const noexcept { return projectDir_; }

    /// Output directory where to write documentation files.
    /// \note If empty, 'docs/' subdirectory of the working directory is assumed.
    const std::filesystem::path& outputDir() const noexcept { return outputDir_; }

    /// Root directory for protobuf built-in '.proto' files ('google/protobuf/descriptor.proto', etc.).
    /// \note This parameter is needed to parse protobuf file when building busrpc project documentation.
    /// \note On *nix systems, '/usr/include' and '/usr/include/local' are implicitly added to the list of directories
    ///       where to search built-in protobuf '.proto' files. However, this directories are only searched if
    ///       file was not found in the command's protobuf root directory.
    const std::filesystem::path& protobufRootDir() const noexcept { return protobufRootDir_; }

private:
    GenDocFormat format_;
    std::filesystem::path projectDir_;
    std::filesystem::path outputDir_;
    std::filesystem::path protobufRootDir_;
};

/// Generate API documentation.
class GenDocCommand: public Command<CommandId::GenDoc, GenDocArgs> {
public:
    /// Base type.
    using BaseType = Command<CommandId::GenDoc, GenDocArgs>;

    /// Create command.
    explicit GenDocCommand(GenDocArgs args) noexcept: BaseType(std::move(args)) { }

protected:
    std::error_code tryExecuteImpl(std::ostream& out, std::ostream& err) const override;
};

/// Define \c gendoc command line options and set a \a callback to be invoked when \a app encounters the command.
void DefineCommand(CLI::App& app, const std::function<void(GenDocArgs)>& callback);
} // namespace busrpc

namespace std {
template<>
struct is_error_code_enum<busrpc::GenDocErrc>: true_type { };
} // namespace std
