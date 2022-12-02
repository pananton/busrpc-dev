#pragma once

#include "commands/command_id.h"

#include <system_error>

/// \file exception.h Busrpc development tool exceptions.

namespace busrpc {

/// Command error.
class command_error: public std::system_error {
public:
    /// Create command error.
    command_error(CommandId commandId, std::error_code ec):
        std::system_error(ec, GetCommandName(commandId)),
        commandId_(commandId)
    { }

    /// Identifier of the failed command.
    CommandId commandId() const noexcept { return commandId_; }

private:
    CommandId commandId_;
};
} // namespace busrpc
