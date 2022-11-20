#pragma once

#include "commands/command.h"

#include <functional>
#include <string>
#include <system_error>

/// \dir commands/gendoc Types and utilites for \c configure command implementation.
/// \file command.h Command \c configure implementation.

namespace CLI {
class App;
}

namespace busrpc {

/// Command-specific error code.
enum class GenDocErrc {
    /// Root directory does not exist.
    Non_Existent_Root_Error = 1,

    /// Busrpc specification is violated.
    Specification_Error = 2,

    /// Failed to parse protobuf file.
    Protobuf_Error = 3,

    /// Failed to create output directory.
    Create_Output_Dir_Error = 4,

    /// Failed to read a protobuf file when building documentation.
    File_Read_Error = 5,

    /// Failed to write generated files with documentation to the output directory.
    File_Write_Error = 6
};

/// Return error category for the \c gendoc command.
const std::error_category& gendoc_error_category();

/// Create error code from the \ref GenDocErrc value.
std::error_code make_error_code(GenDocErrc errc);

/// Target language for which files are configured.
enum class GenDocFormat {
    /// JSON.
    Json = 1
};

constexpr const char* GetGenDocFormatStr(GenDocFormat lang)
{
    switch (lang) {
    case GenDocFormat::Json: return "json";
    default: return nullptr;
    }
}

/// Arguments of the \c gendoc command.
struct GenDocArgs {
    /// Create \c gendoc command arguments.
    GenDocArgs(GenDocFormat format, std::string rootDir = {}, std::string outputDir = {});

    /// Documentation format (required).
    GenDocFormat format;

    /// Busrpc root directory.
    /// \note If empty, working directory is assumed.
    std::string rootDir = "";

    /// Output directory.
    /// \note If empty, '_docs/' subdirectory of the working directory is assumed.
    std::string outputDir = "";
};

/// Generate API documentation.
class GenDocCommand: public Command<CommandId::GenDoc, GenDocArgs> {
public:
    /// Base type.
    using BaseType = Command<CommandId::GenDoc, GenDocArgs>;

    /// Create command.
    GenDocCommand(GenDocArgs args) noexcept: BaseType(std::move(args)) { }

protected:
    std::error_code tryExecuteImpl(std::ostream& out, std::ostream& err) const override;
};

/// Define \c configure command line options and set a \a callback to be invoked when \a app encounters the command.
void DefineCommand(CLI::App& app, const std::function<void(GenDocArgs)>& callback);
} // namespace busrpc

namespace std {
template<>
struct is_error_code_enum<busrpc::GenDocErrc>: true_type { };
} // namespace std
