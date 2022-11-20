#include "commands/check/check_command.h"

#include <string>

namespace busrpc {

namespace {
class CheckErrorCategory: public std::error_category {
public:
    const char* name() const noexcept override { return "check"; }

    std::string message(int code) const override
    {
        switch (static_cast<CheckErrc>(code)) {
        case CheckErrc::Non_Existent_Root_Error: return "root directory does not exist";
        case CheckErrc::Specification_Error: return "busrpc specification is violated";
        case CheckErrc::Docs_Error: return "undocumented entities found";
        case CheckErrc::Style_Error: return "busrpc protobuf style guide is violated";
        case CheckErrc::Protobuf_Error: return "protobuf parsing error";
        case CheckErrc::File_Read_Error: return "failed to read file";
        default: return "unknown error";
        }
    }

    bool equivalent(int code, const std::error_condition& condition) const noexcept override
    {
        switch (static_cast<CheckErrc>(code)) {
        case CheckErrc::Non_Existent_Root_Error: return condition == CommandError::Argument_Error;
        case CheckErrc::Specification_Error: return condition == CommandError::Logic_Error;
        case CheckErrc::Docs_Error: return condition == CommandError::Logic_Error;
        case CheckErrc::Style_Error: return condition == CommandError::Logic_Error;
        case CheckErrc::Protobuf_Error: return condition == CommandError::Protobuf_Error;
        case CheckErrc::File_Read_Error: return condition == CommandError::File_Access_Error;
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
