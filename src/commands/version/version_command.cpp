#include "commands/version/version_command.h"
#include "configure.h"

#include <string>

namespace busrpc {

namespace {
class VersionErrorCategory: public std::error_category {
public:
    const char* name() const noexcept override { return "version"; }
    std::string message(int) const override { return "unknown error"; }
    bool equivalent(int, const std::error_condition&) const noexcept override { return false; }
};

} // namespace

std::error_code VersionCommand::tryExecuteImpl(std::ostream& out, std::ostream&) const
{
    out << "Busrpc development tool version " << BUSRPC_VERSION << std::endl;
    return {0, version_error_category()};
}

const std::error_category& version_error_category()
{
    static const VersionErrorCategory category;
    return category;
}

std::error_code make_error_code(VersionErrc e)
{
    return {static_cast<int>(e), version_error_category()};
}
} // namespace busrpc
