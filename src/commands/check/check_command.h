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
    /// Busrpc protobuf style violated.
    /// \note This code only returned if command is executed with flag, indicating that warnings should be treated
    ///       as errors.
    Style_Violated = 1,

    /// Busrpc documentation rule violated.
    /// \note This code only returned if command is executed with flag, indicating that warnings should be treated
    ///       as errors.
    Doc_Rule_Violated = 2,

    /// Busrpc specification violated.
    Spec_Violated = 3,

    /// Failed to parse protobuf file.
    Protobuf_Parsing_Failed = 4,

    /// Failed to read file to be checked.
    File_Read_Failed = 5,

    /// Busrpc project directory does not exist or does not represent a valid project directory.
    Invalid_Project_Dir = 6
};

/// Return error category for the \c check command.
const std::error_category& check_error_category();

/// Create error code from the \ref CheckErrc value.
std::error_code make_error_code(CheckErrc errc);

/// Arguments of the \c check command.
class CheckArgs {
public:
    /// Create \c check command arguments.
    CheckArgs(std::string projectDir = {},
              bool skipDocsChecks = false,
              bool skipStyleChecks = false,
              bool warningAsError = false):
        projectDir_(std::move(projectDir)),
        skipDocsChecks_(skipDocsChecks),
        skipStyleChecks_(skipStyleChecks),
        warningAsError_(warningAsError)
    { }

    /// Busrpc project directory (the one containing 'api/' and 'services/' subdirectories).
    /// \note If empty, working directory is assumed.
    const std::string& projectDir() const noexcept { return projectDir_; }

    /// Flag indicating whether documentation rules check should be skipped.
    bool skipDocsChecks() const noexcept { return skipDocsChecks_; }

    /// Flag indicating whether documentation rules check should be skipped.
    bool skipStyleChecks() const noexcept { return skipStyleChecks_; }

    /// Flag indicating whether warnings should be treated as errors.
    bool warningAsError() const noexcept { return warningAsError_; }

private:
    std::string projectDir_;
    bool skipDocsChecks_;
    bool skipStyleChecks_;
    bool warningAsError_;
};

/// Check API for conformance to the busrpc specification.
class CheckCommand: public Command<CommandId::Check, CheckArgs> {
public:
    /// Base type.
    using BaseType = Command<CommandId::Check, CheckArgs>;

    /// Create command.
    explicit CheckCommand(CheckArgs args) noexcept: BaseType(std::move(args)) { }

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
