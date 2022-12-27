#include "commands/check/check_command.h"

#include <string>

namespace busrpc {

namespace {
class CheckErrorCategory: public std::error_category {
public:
    const char* name() const noexcept override { return "check"; }

    std::string message(int code) const override
    {
        using enum CheckErrc;

        switch (static_cast<CheckErrc>(code)) {
        case Style_Violated: return "Busrpc protobuf style violated";
        case Doc_Rule_Violated: return "Busrpc documentation rule violated";
        case Spec_Violated: return "Busrpc specification violated";
        case Protobuf_Parsing_Failed: return "Failed to parse protobuf file";
        case File_Read_Failed: return "Failed to read file";
        case Invalid_Project_Dir: return "Invalid busrpc project directory";
        default: return "Unknown error";
        }
    }

    bool equivalent(int code, const std::error_condition& condition) const noexcept override
    {
        using enum CheckErrc;

        switch (static_cast<CheckErrc>(code)) {
        case Style_Violated: return condition == CommandError::Spec_Violated;
        case Doc_Rule_Violated: return condition == CommandError::Spec_Violated;
        case Spec_Violated: return condition == CommandError::Spec_Violated;
        case Protobuf_Parsing_Failed: return condition == CommandError::Protobuf_Parsing_Failed;
        case File_Read_Failed: return condition == CommandError::File_Operation_Failed;
        case Invalid_Project_Dir: return condition == CommandError::Invalid_Argument;
        default: return false;
        }
    }
};
} // namespace

std::error_code CheckCommand::tryExecuteImpl(std::ostream&, std::ostream&) const
{
    return {0, check_error_category()};
}

const std::error_category& check_error_category()
{
    static const CheckErrorCategory category;
    return category;
}

std::error_code make_error_code(CheckErrc e)
{
    return {static_cast<int>(e), check_error_category()};
}
} // namespace busrpc
