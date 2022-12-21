#include "commands/gendoc/gendoc_command.h"

#include <string>

namespace busrpc {

namespace {
class GenDocErrorCategory: public std::error_category {
public:
    const char* name() const noexcept override { return "gendoc"; }

    std::string message(int code) const override
    {
        using enum GenDocErrc;

        switch (static_cast<GenDocErrc>(code)) {
        case File_Write_Failed: return "Failed to write file";
        case File_Read_Failed: return "Failed to read file";
        case Create_Output_Dir_Failed: return "Failed to create output directory";
        case Protobuf_Parsing_Failed: return "Failed to parse protobuf file";
        case Spec_Violated: return "Busrpc specification is violated";
        case Project_Dir_Does_Not_Exist: return "Busrpc project directory does not exist";
        default: return "Unknown error";
        }
    }

    bool equivalent(int code, const std::error_condition& condition) const noexcept override
    {
        using enum GenDocErrc;

        switch (static_cast<GenDocErrc>(code)) {
        case File_Write_Failed: return condition == CommandError::File_Operation_Failed;
        case File_Read_Failed: return condition == CommandError::File_Operation_Failed;
        case Create_Output_Dir_Failed: return condition == CommandError::File_Operation_Failed;
        case Protobuf_Parsing_Failed: return condition == CommandError::Protobuf_Parsing_Failed;
        case Spec_Violated: return condition == CommandError::Spec_Violated;
        case Project_Dir_Does_Not_Exist: return condition == CommandError::Invalid_Argument;
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
