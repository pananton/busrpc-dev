#include "commands/configure/configure_command.h"

#include <string>

namespace busrpc {

namespace {
class ConfigureErrorCategory: public std::error_category {
public:
    const char* name() const noexcept override { return "configure"; }

    std::string message(int code) const override
    {
        switch (static_cast<ConfigureErrc>(code)) {
        case ConfigureErrc::File_Write_Error: return "failed to write file";
        case ConfigureErrc::File_Read_Error: return "failed to read file";
        case ConfigureErrc::Create_Output_Dir_Error: return "failed to create output directory";
        case ConfigureErrc::Protobuf_Error: return "protobuf parsing error";
        case ConfigureErrc::File_Not_Found: return "file not found";
        case ConfigureErrc::Non_Existent_Root_Error: return "root directory does not exist";
        default: return "unknown error";
        }
    }

    bool equivalent(int code, const std::error_condition& condition) const noexcept override
    {
        switch (static_cast<ConfigureErrc>(code)) {
        case ConfigureErrc::File_Write_Error: return condition == CommandError::File_Access_Error;
        case ConfigureErrc::File_Read_Error: return condition == CommandError::File_Access_Error;
        case ConfigureErrc::Create_Output_Dir_Error: return condition == CommandError::File_Access_Error;
        case ConfigureErrc::Protobuf_Error: return condition == CommandError::Protobuf_Error;
        case ConfigureErrc::File_Not_Found: return condition == CommandError::Argument_Error;
        case ConfigureErrc::Non_Existent_Root_Error: return condition == CommandError::Argument_Error;
        default: return false;
        }
    }
};
} // namespace

ConfigureArgs::ConfigureArgs(ConfigureLang lang,
                             std::vector<std::string> files,
                             std::string rootDir,
                             std::string outputDir):
    lang(lang),
    files(std::move(files)),
    rootDir(std::move(rootDir)),
    outputDir(std::move(outputDir))
{ }

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
