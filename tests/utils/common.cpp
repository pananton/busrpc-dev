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

std::vector<std::string> SplitByNewline(const std::string& str)
{
    std::stringstream s(str);
    std::string line;
    std::vector<std::string> result;

    while (std::getline(s, line, '\n')) {
        if (!line.empty()) {
            result.push_back(std::move(line));
        }
    }

    return result;
}
}} // namespace busrpc::test
