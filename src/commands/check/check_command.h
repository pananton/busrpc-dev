#pragma once

#include "commands/command.h"

#include <filesystem>
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
    CheckArgs(std::filesystem::path projectDir = std::filesystem::current_path(),
              std::filesystem::path protobufRootDir = {},
              bool ignoreSpecWarnings = false,
              bool ignoreDocWarnings = false,
              bool ignoreStyleWarnings = false,
              bool warningAsError = false):
        projectDir_(std::move(projectDir)),
        protobufRootDir_(std::move(protobufRootDir)),
        ignoreSpecWarnings_(ignoreSpecWarnings),
        ignoreDocWarnings_(ignoreDocWarnings),
        ignoreStyleWarnings_(ignoreStyleWarnings),
        warningAsError_(warningAsError)
    { }

    /// Busrpc project directory.
    /// \note If empty, working directory is assumed.
    const std::filesystem::path& projectDir() const noexcept { return projectDir_; }

    /// Root directory for protobuf built-in '.proto' files ('google/protobuf/descriptor.proto', etc.).
    /// \note On *nix systems, '/usr/include' and '/usr/include/local' are implicitly added to the list of directories
    ///       where to search built-in protobuf '.proto' files. However, this directories are only searched if
    ///       file was not found in the command's protobuf root directory.
    const std::filesystem::path& protobufRootDir() const noexcept { return protobufRootDir_; }

    /// Flag indicating whether busrpc specification warnings should be ignored.
    /// \note Ignored warnings are not printed to the command output and do not affect it's final result.
    bool ignoreSpecWarnings() const noexcept { return ignoreSpecWarnings_; }

    /// Flag indicating whether warnings related to the project documentation should be ignored.
    /// \note Ignored warnings are not printed to the command output and do not affect it's final result.
    bool ignoreDocWarnings() const noexcept { return ignoreDocWarnings_; }

    /// Flag indicating whether warnings related to project protobuf style should be ignored.
    /// \note Ignored warnings are not printed to the command output and do not affect command result.
    bool ignoreStyleWarnings() const noexcept { return ignoreStyleWarnings_; }

    /// Flag indicating whether warnings should be treated as errors.
    bool warningAsError() const noexcept { return warningAsError_; }

private:
    std::filesystem::path projectDir_;
    std::filesystem::path protobufRootDir_;
    bool ignoreSpecWarnings_;
    bool ignoreDocWarnings_;
    bool ignoreStyleWarnings_;
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
