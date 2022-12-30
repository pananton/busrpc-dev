#include "commands/command.h"

#include <sstream>

namespace busrpc {

namespace {

struct CommandErrorCategory: std::error_category {
    const char* name() const noexcept override { return "command"; }

    std::string message(int ev) const override
    {
        switch (static_cast<CommandError>(ev)) {
        case CommandError::Spec_Violated: return "Busrpc specification violated";
        case CommandError::Protobuf_Parsing_Failed: return "Failed to parse protobuf file";
        case CommandError::File_Operation_Failed: return "File or directory access error";
        case CommandError::Invalid_Argument: return "Invalid argument";
        default: return "Unknown error";
        }
    }
};
} // namespace

void CommandBase::execute(std::ostream* out, std::ostream* err) const
{
    auto ec = tryExecute(out, err);

    if (ec) {
        throw command_error(id_, ec);
    }
}

std::error_code CommandBase::tryExecute(std::ostream* out, std::ostream* err) const
{
    std::ostringstream outNull, errNull;

    if (!out) {
        out = &outNull;
    }

    if (!err) {
        err = &errNull;
    }

    return tryExecuteImpl(*out, *err);
}

const std::error_category& command_error_category()
{
    static const CommandErrorCategory category;
    return category;
}

std::error_condition make_error_condition(CommandError e)
{
    return {static_cast<int>(e), command_error_category()};
}
} // namespace busrpc
