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
    /// Root directory does not exist.
    Non_Existent_Root_Error = 1,

    /// File which is part of the dependency tree is not found.
    File_Not_Found = 2,

    /// Failed to parse protobuf file.
    Protobuf_Error = 3,

    /// Failed to read file which is part of the dependency tree.
    File_Read_Error = 4
};

/// Return error category for the \c imports command.
const std::error_category& imports_error_category();

/// Create error code from the \ref ImportsErrc value.
std::error_code make_error_code(ImportsErrc errc);

/// Arguments of the \c imports command.
struct ImportsArgs {
    /// Files which dependencies to output (should be specified relatively to the busrpc root directory).
    std::vector<std::string> files = {};

    /// Busrpc root directory.
    /// \note If empty, working directory is assumed.
    std::string rootDir = "";
};

/// Output files directly or indirectly imported by the specified file(s).
/// \note Files are outputted by their nesting level in the directory hierarchy (from most nested files to files
///       from the root directory).
class ImportsCommand: public Command<CommandId::Imports, ImportsArgs> {
public:
    /// Base type.
    using BaseType = Command<CommandId::Imports, ImportsArgs>;

    /// Create command.
    ImportsCommand(ImportsArgs args) noexcept: BaseType(std::move(args)) { }

protected:
    std::error_code tryExecuteImpl(std::ostream& out, std::ostream& err) const override;
};

/// Define \c check command line options and set a \a callback to be invoked when \a app encounters the command.
void DefineCommand(CLI::App& app, const std::function<void(ImportsArgs)>& callback);
} // namespace busrpc

namespace std {
template<>
struct is_error_code_enum<busrpc::ImportsErrc>: true_type { };
} // namespace std
