#pragma once

#include "commands/command.h"

#include <functional>
#include <system_error>

/// \dir commands/version Types and utilites for \c version command implementation.
/// \file version_command.h Command \c version implementation.

namespace CLI {
class App;
}

namespace busrpc {

/// Command-specific error code.
enum class VersionErrc {};

/// Return error category for the \c version command.
const std::error_category& version_error_category();

/// Create error code from the \ref VersionErrc value.
std::error_code make_error_code(VersionErrc errc);

/// Arguments of the \c version command.
class VersionArgs { };

/// Output busrpc development tool version.
class VersionCommand: public Command<CommandId::Version, VersionArgs> {
public:
    /// Base type.
    using BaseType = Command<CommandId::Version, VersionArgs>;

    /// Create command.
    explicit VersionCommand(VersionArgs args) noexcept: BaseType(std::move(args)) { }

protected:
    /// Execute command.
    std::error_code tryExecuteImpl(std::ostream& out, std::ostream& err) const override;
};

/// Define \c version command line options and set a \a callback to be invoked when \a app encounters the command.
void DefineCommand(CLI::App& app, const std::function<void(VersionArgs)>& callback);
} // namespace busrpc

namespace std {
template<>
struct is_error_code_enum<busrpc::VersionErrc>: true_type { };
} // namespace std
