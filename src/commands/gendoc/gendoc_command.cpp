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
        case Spec_Violated: return "Busrpc specification violated";
        case Protobuf_Parsing_Failed: return "Failed to parse protobuf file";
        case File_Write_Failed: return "Failed to write file";
        case File_Read_Failed: return "Failed to read file";
        case Invalid_Project_Dir: return "Invalid busrpc project directory";
        default: return "Unknown error";
        }
    }

    bool equivalent(int code, const std::error_condition& condition) const noexcept override
    {
        using enum GenDocErrc;

        switch (static_cast<GenDocErrc>(code)) {
        case Spec_Violated: return condition == CommandError::Spec_Violated;
        case Protobuf_Parsing_Failed: return condition == CommandError::Protobuf_Parsing_Failed;
        case File_Write_Failed: return condition == CommandError::File_Operation_Failed;
        case File_Read_Failed: return condition == CommandError::File_Operation_Failed;
        case Invalid_Project_Dir: return condition == CommandError::Invalid_Argument;
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
