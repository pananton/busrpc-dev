#pragma once

#include "commands/command.h"

#include <functional>
#include <optional>
#include <system_error>

/// \dir commands/help Types and utilites for \c help command implementation.
/// \file help_command.h Command \c help implementation.

namespace CLI {
class App;
}

namespace busrpc {

/// Command-specific error code.
enum class HelpErrc {};

/// Return error category for the \c help command.
const std::error_category& help_error_category();

/// Create error code from the \ref HelpErrc value.
std::error_code make_error_code(HelpErrc errc);

/// Arguments of the \c help command.
class HelpArgs {
public:
    /// Create \c help command arguments.
    HelpArgs(std::optional<CommandId> commandId = std::nullopt): commandId_(std::move(commandId)) { }

    /// Identifier of the command for which help message should be outputted.
    /// \note If not set, then command outputs short information about all supported commands.
    const std::optional<CommandId>& commandId() const noexcept { return commandId_; }

private:
    std::optional<CommandId> commandId_;
};

/// Output information about command.
class HelpCommand: public Command<CommandId::Help, HelpArgs> {
public:
    /// Base type.
    using BaseType = Command<CommandId::Help, HelpArgs>;

    /// Create command.
    explicit HelpCommand(HelpArgs args) noexcept: BaseType(std::move(args)) { }

protected:
    /// Execute command.
    std::error_code tryExecuteImpl(std::ostream& out, std::ostream& err) const override;
};

/// Define \c help command line options and set a \a callback to be invoked when \a app encounters the command.
void DefineCommand(CLI::App& app, const std::function<void(HelpArgs)>& callback);
} // namespace busrpc

namespace std {
template<>
struct is_error_code_enum<busrpc::HelpErrc>: true_type { };
} // namespace std
