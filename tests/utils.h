#pragma once

#include "commands/command_id.h"
#include "exception.h"

#include <optional>
#include <string>

#define EXPECT_COMMAND_EXCEPTION(OP, CODE)     \
    try {                                      \
        (OP);                                  \
        ADD_FAILURE() << "No exception";       \
    } catch (const busrpc::command_error& e) { \
        EXPECT_EQ(e.code(), (CODE));           \
    }

namespace busrpc {

bool IsHelpMessage(const std::string& msg, std::optional<CommandId> commandId = std::nullopt);
}
