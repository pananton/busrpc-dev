#pragma once

#include "commands/command.h"

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
    /// Failed to write generated documentation to the output directory.
    File_Write_Failed = 1,

    /// Failed to read a source file for documentation.
    File_Read_Failed = 2,

    /// Failed to create output directory.
    Create_Output_Dir_Failed = 3,

    /// Failed to parse protobuf file.
    Protobuf_Parsing_Failed = 4,

    /// Busrpc specification is violated.
    Spec_Violated = 5,

    /// Busrpc project directory does not exist.
    Project_Dir_Does_Not_Exist = 6
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
    GenDocArgs(GenDocFormat format, std::string projectDir = {}, std::string outputDir = {}):
        format_(format),
        projectDir_(std::move(projectDir)),
        outputDir_(std::move(outputDir))
    { }

    /// Format of the documentation.
    GenDocFormat format() const noexcept { return format_; }

    /// Busrpc project directory (the one containing 'api/' and 'services/' subdirectories).
    /// \note If empty, working directory is assumed.
    const std::string& projectDir() const noexcept { return projectDir_; }

    /// Output directory where to write documentation files.
    /// \note If empty, 'docs/' subdirectory of the working directory is assumed.
    const std::string& outputDir() const noexcept { return outputDir_; }

private:
    GenDocFormat format_;
    std::string projectDir_;
    std::string outputDir_;
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
