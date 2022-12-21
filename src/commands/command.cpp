#include "commands/command.h"

#include <sstream>

namespace busrpc {

namespace {
struct CommandErrorCategory: std::error_category {
    const char* name() const noexcept override { return "command"; }

    std::string message(int ev) const override
    {
        using enum CommandError;

        switch (static_cast<CommandError>(ev)) {
        case Internal: return "internal error";
        case File_Operation_Failed: return "file or directory access error";
        case Protobuf_Parsing_Failed: return "failed to parse protobuf file";
        case Spec_Violated: return "busrpc specification violated";
        case Invalid_Argument: return "invalid argument";
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
