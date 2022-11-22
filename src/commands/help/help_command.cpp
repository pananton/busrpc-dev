#include "commands/help/help_command.h"
#include "app.h"

#include <CLI/CLI.hpp>

#include <string>

namespace busrpc {

namespace {
class HelpErrorCategory: public std::error_category {
public:
    const char* name() const noexcept override { return "help"; }
    std::string message(int) const override { return "unknown error"; }
    bool equivalent(int, const std::error_condition&) const noexcept override { return false; }
};
} // namespace

std::error_code HelpCommand::tryExecuteImpl(std::ostream& out, std::ostream&) const
{
    CLI::App app;
    InitApp(app);
    const char* commandName = args().commandId ? GetCommandName(*(args().commandId)) : nullptr;
    CLI::App* appCommand = commandName ? app.get_subcommand(commandName) : &app;

    out << appCommand->help() << std::endl;
    return std::error_code(0, help_error_category());
}

const std::error_category& help_error_category()
{
    static const HelpErrorCategory category;
    return category;
}

std::error_code make_error_code(HelpErrc e)
{
    return {static_cast<int>(e), help_error_category()};
}
} // namespace busrpc
