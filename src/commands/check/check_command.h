#pragma once

#include "commands/command.h"

#include <functional>
#include <string>
#include <system_error>

/// \dir commands/check Types and utilites for \c check command implementation.
/// \file check_command.h Command \c check implementation.

namespace CLI {
class App;
}

namespace busrpc {

/// Command-specific error code.
enum class CheckErrc {
    /// Root directory does not exist.
    Non_Existent_Root_Error = 1,

    /// Violation(s) of busrpc specification detected.
    Specification_Error = 2,

    /// Violation(s) of documentation rules detected.
    /// \note This code only returned if command is executed with flag, indicating that warnings should be treated
    ///       as errors.
    Docs_Error = 3,

    /// Violation(s) of style rules detected.
    /// \note This code only returned if command is executed with flag, indicating that warnings should be treated
    ///       as errors.
    Style_Error = 4,

    /// Failed to parse protobuf file.
    Protobuf_Error = 5,

    /// Failed to read file to be checked.
    File_Read_Error = 6
};

/// Return error category for the \c check command.
const std::error_category& check_error_category();

/// Create error code from the \ref CheckErrc value.
std::error_code make_error_code(CheckErrc errc);

/// Arguments of the \c check command.
struct CheckArgs {
    /// Busrpc root directory (the one containing 'api/' and 'services/' subdirectories).
    /// \note If empty, working directory is assumed.
    std::string rootDir = "";

    /// Skip checks that verify API entities are documented.
    bool skip_docs_checks = false;

    /// Skip protobuf code style checks.
    bool skip_style_checks = false;

    /// Treat warnings as errors.
    bool warning_as_error = false;
};

/// Check API for conformance to the busrpc specification.
class CheckCommand: public Command<CommandId::Check, CheckArgs> {
public:
    /// Base type.
    using BaseType = Command<CommandId::Check, CheckArgs>;

    /// Create command.
    CheckCommand(CheckArgs args) noexcept: BaseType(std::move(args)) { }

protected:
    std::error_code tryExecuteImpl(std::ostream& out, std::ostream& err) const override;
};

/// Define \c check command line options and set a \a callback to be invoked when \a app encounters the command.
void DefineCommand(CLI::App& app, const std::function<void(CheckArgs)>& callback);
} // namespace busrpc

namespace std {
template<>
struct is_error_code_enum<busrpc::CheckErrc>: true_type { };
} // namespace std
