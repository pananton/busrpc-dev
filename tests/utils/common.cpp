#include "utils/common.h"

#include <algorithm>
#include <sstream>
#include <string>

namespace busrpc { namespace test {

bool IsHelpMessage(const std::string& msg, std::optional<CommandId> commandId)
{
    auto pos = msg.find("Usage:");

    if (pos == std::string::npos) {
        return false;
    }

    if (commandId) {
        auto lineEnd = msg.find('\n', pos);
        std::string usageLine = msg.substr(pos, lineEnd != std::string::npos ? lineEnd - pos : lineEnd);
        return usageLine.find(GetCommandName(*commandId)) != std::string::npos;
    } else {
        return msg.find("Subcommands:") != std::string::npos;
    }
}
}} // namespace busrpc::test
