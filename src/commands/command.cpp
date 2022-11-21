#include "commands/command.h"

#include <sstream>

namespace busrpc {

namespace {
struct CommandErrorCategory: std::error_category {
    const char* name() const noexcept override { return "command"; }

    std::string message(int ev) const override
    {
        switch (static_cast<CommandError>(ev)) {
        case CommandError::Argument_Error: return "invalid argument";
        case CommandError::Logic_Error: return "logic error";
        case CommandError::Protobuf_Error: return "protobuf parsing error";
        case CommandError::File_Access_Error: return "file or directory access error";
        case CommandError::Internal_Error: return "internal error";
        default: return "unknown error";
        }
    }
};
} // namespace

void CommandBase::execute(std::optional<std::reference_wrapper<std::ostream>> out,
                          std::optional<std::reference_wrapper<std::ostream>> err) const
{
    auto ec = tryExecute(std::move(out), std::move(err));

    if (ec) {
        throw command_error(id_, ec);
    }
}

std::error_code CommandBase::tryExecute(std::optional<std::reference_wrapper<std::ostream>> out,
                                        std::optional<std::reference_wrapper<std::ostream>> err) const
{
    std::ostringstream outNull, errNull;

    if (!out) {
        out.emplace(outNull);
    }

    if (!err) {
        err.emplace(errNull);
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
