#pragma once

#include "commands/command_id.h"

#include <system_error>

/// \file exception.h Busrpc development tool exceptions.

namespace busrpc {

/// Command error.
class command_error: public std::system_error {
public:
    /// Create command error.
    command_error(CommandId command_id, std::error_code ec):
        std::system_error(ec, GetCommandName(command_id)),
        command_id_(command_id)
    { }

    /// Return failed command identifier.
    CommandId command_id() const noexcept { return command_id_; }

private:
    CommandId command_id_;
};
} // namespace busrpc
