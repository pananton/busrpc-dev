#pragma once

#include "commands/command.h"

#include <functional>
#include <string>
#include <system_error>
#include <vector>

/// \dir commands/imports Types and utilites for \c imports command implementation.
/// \file imports_command.h Command \c imports implementation.

namespace CLI {
class App;
}

namespace busrpc {

/// Command-specific error code.
enum class ImportsErrc {
    /// Failed to read file for which import tree is built.
    File_Read_Failed = 1,

    /// Failed to parse protobuf file.
    Protobuf_Parsing_Failed = 2,

    /// File for which import tree should be built is not found.
    File_Not_Found = 3,

    /// Busrpc root directory does not exist.
    Root_Does_Not_Exist = 4
};

/// Return error category for the \c imports command.
const std::error_category& imports_error_category();

/// Create error code from the \ref ImportsErrc value.
std::error_code make_error_code(ImportsErrc errc);

/// Arguments of the \c imports command.
struct ImportsArgs {
    /// Files which imports to output (should be nested in the busrpc root directory).
    std::vector<std::string> files = {};

    /// Busrpc root directory (the one containing 'api/' and 'services/' subdirectories).
    /// \note If empty, working directory is assumed.
    std::string rootDir = "";

    /// Only output paths to the dependencies, do not output paths to \ref files themselves.
    bool only_deps = false;
};

/// Output relative paths to the files directly or indirectly imported by the specified file(s).
class ImportsCommand: public Command<CommandId::Imports, ImportsArgs> {
public:
    /// Base type.
    using BaseType = Command<CommandId::Imports, ImportsArgs>;

    /// Create command.
    ImportsCommand(ImportsArgs args) noexcept: BaseType(std::move(args)) { }

protected:
    std::error_code tryExecuteImpl(std::ostream& out, std::ostream& err) const override;
};

/// Define \c imports command line options and set a \a callback to be invoked when \a app encounters the command.
void DefineCommand(CLI::App& app, const std::function<void(ImportsArgs)>& callback);
} // namespace busrpc

namespace std {
template<>
struct is_error_code_enum<busrpc::ImportsErrc>: true_type { };
} // namespace std
