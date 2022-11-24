#pragma once

#include "commands/command_id.h"
#include "exception.h"

#include <optional>
#include <string>
#include <vector>

#define EXPECT_COMMAND_EXCEPTION(OP, CODE)     \
    try {                                      \
        (OP);                                  \
        ADD_FAILURE() << "No exception";       \
    } catch (const busrpc::command_error& e) { \
        EXPECT_EQ(e.code(), (CODE));           \
    }

namespace busrpc { namespace test {

bool IsHelpMessage(const std::string& msg, std::optional<CommandId> commandId = std::nullopt);
std::vector<std::string> SplitByNewline(const std::string& str);
}} // namespace busrpc::test
