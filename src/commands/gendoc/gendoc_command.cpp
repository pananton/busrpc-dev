#include "commands/gendoc/gendoc_command.h"

#include <string>

namespace busrpc {

namespace {
class GenDocErrorCategory: public std::error_category {
public:
    const char* name() const noexcept override { return "gendoc"; }

    std::string message(int code) const override
    {
        switch (static_cast<GenDocErrc>(code)) {
        case GenDocErrc::File_Write_Error: return "failed to write file";
        case GenDocErrc::File_Read_Error: return "failed to read file";
        case GenDocErrc::Create_Output_Dir_Error: return "failed to create output directory";
        case GenDocErrc::Protobuf_Error: return "protobuf parsing error";
        case GenDocErrc::Specification_Error: return "busrpc specification is violated";
        case GenDocErrc::Non_Existent_Root_Error: return "root directory does not exist";
        default: return "unknown error";
        }
    }

    bool equivalent(int code, const std::error_condition& condition) const noexcept override
    {
        switch (static_cast<GenDocErrc>(code)) {
        case GenDocErrc::File_Write_Error: return condition == CommandError::File_Access_Error;
        case GenDocErrc::File_Read_Error: return condition == CommandError::File_Access_Error;
        case GenDocErrc::Create_Output_Dir_Error: return condition == CommandError::File_Access_Error;
        case GenDocErrc::Protobuf_Error: return condition == CommandError::Protobuf_Error;
        case GenDocErrc::Specification_Error: return condition == CommandError::Logic_Error;
        case GenDocErrc::Non_Existent_Root_Error: return condition == CommandError::Argument_Error;
        default: return false;
        }
    }
};
} // namespace

GenDocArgs::GenDocArgs(GenDocFormat format, std::string rootDir, std::string outputDir):
    format(format),
    rootDir(std::move(rootDir)),
    outputDir(std::move(outputDir))
{ }

std::error_code GenDocCommand::tryExecuteImpl(std::ostream&, std::ostream&) const
{
    return {0, gendoc_error_category()};
}

const std::error_category& gendoc_error_category()
{
    static const GenDocErrorCategory category;
    return category;
}

std::error_code make_error_code(GenDocErrc e)
{
    return {static_cast<int>(e), gendoc_error_category()};
}
} // namespace busrpc
