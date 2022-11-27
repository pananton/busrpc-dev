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
        case GenDocErrc::File_Write_Failed: return "Failed to write file";
        case GenDocErrc::File_Read_Failed: return "Failed to read file";
        case GenDocErrc::Create_Output_Dir_Failed: return "Failed to create output directory";
        case GenDocErrc::Protobuf_Parsing_Failed: return "Failed to parse protobuf file";
        case GenDocErrc::Spec_Violated: return "Busrpc specification is violated";
        case GenDocErrc::Root_Does_Not_Exist: return "Busrpc root directory does not exist";
        default: return "Unknown error";
        }
    }

    bool equivalent(int code, const std::error_condition& condition) const noexcept override
    {
        switch (static_cast<GenDocErrc>(code)) {
        case GenDocErrc::File_Write_Failed: return condition == CommandError::File_Operation_Failed;
        case GenDocErrc::File_Read_Failed: return condition == CommandError::File_Operation_Failed;
        case GenDocErrc::Create_Output_Dir_Failed: return condition == CommandError::File_Operation_Failed;
        case GenDocErrc::Protobuf_Parsing_Failed: return condition == CommandError::Protobuf_Parsing_Failed;
        case GenDocErrc::Spec_Violated: return condition == CommandError::Spec_Violated;
        case GenDocErrc::Root_Does_Not_Exist: return condition == CommandError::Invalid_Argument;
        default: return false;
        }
    }
};
} // namespace

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
