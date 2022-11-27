#include "commands/configure/configure_command.h"

#include <cassert>
#include <string>

namespace busrpc {

namespace {
class ConfigureErrorCategory: public std::error_category {
public:
    const char* name() const noexcept override { return "configure"; }

    std::string message(int code) const override
    {
        switch (static_cast<ConfigureErrc>(code)) {
        case ConfigureErrc::File_Write_Failed: return "Failed to write file";
        case ConfigureErrc::File_Read_Failed: return "Failed to read file";
        case ConfigureErrc::Create_Output_Dir_Failed: return "Failed to create output directory";
        case ConfigureErrc::Protobuf_Parsing_Failed: return "Failed to parse protobuf file";
        case ConfigureErrc::File_Not_Found: return "File not found";
        case ConfigureErrc::Root_Does_Not_Exist: return "Busrpc root directory does not exist";
        default: return "Unknown error";
        }
    }

    bool equivalent(int code, const std::error_condition& condition) const noexcept override
    {
        switch (static_cast<ConfigureErrc>(code)) {
        case ConfigureErrc::File_Write_Failed: return condition == CommandError::File_Operation_Failed;
        case ConfigureErrc::File_Read_Failed: return condition == CommandError::File_Operation_Failed;
        case ConfigureErrc::Create_Output_Dir_Failed: return condition == CommandError::File_Operation_Failed;
        case ConfigureErrc::Protobuf_Parsing_Failed: return condition == CommandError::Protobuf_Parsing_Failed;
        case ConfigureErrc::File_Not_Found: return condition == CommandError::Invalid_Argument;
        case ConfigureErrc::Root_Does_Not_Exist: return condition == CommandError::Invalid_Argument;
        default: return false;
        }
    }
};
} // namespace

ConfigureLang ConfigureArgs::lang() const noexcept
{
    switch (options_.index()) {
    case 0: return ConfigureLang::Java;
    default: assert(false); return static_cast<ConfigureLang>(0);
    }
}

std::error_code ConfigureCommand::tryExecuteImpl(std::ostream&, std::ostream&) const
{
    return {0, configure_error_category()};
}

const std::error_category& configure_error_category()
{
    static const ConfigureErrorCategory category;
    return category;
}

std::error_code make_error_code(ConfigureErrc e)
{
    return {static_cast<int>(e), configure_error_category()};
}
} // namespace busrpc
