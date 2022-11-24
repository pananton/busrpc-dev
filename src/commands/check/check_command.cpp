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
        case CheckErrc::File_Read_Failed: return "failed to read file";
        case CheckErrc::Protobuf_Parsing_Failed: return "failed to parse protobuf file";
        case CheckErrc::Protobuf_Style_Violated: return "busrpc protobuf style violated";
        case CheckErrc::Undocumeted_Entity: return "undocumented entities detected";
        case CheckErrc::Spec_Violated: return "busrpc specification violated";
        case CheckErrc::Root_Does_Not_Exist: return "busrpc root directory does not exist";
        default: return "unknown error";
        }
    }

    bool equivalent(int code, const std::error_condition& condition) const noexcept override
    {
        switch (static_cast<CheckErrc>(code)) {
        case CheckErrc::File_Read_Failed: return condition == CommandError::File_Operation_Failed;
        case CheckErrc::Protobuf_Parsing_Failed: return condition == CommandError::Protobuf_Parsing_Failed;
        case CheckErrc::Protobuf_Style_Violated: return condition == CommandError::Spec_Violated;
        case CheckErrc::Undocumeted_Entity: return condition == CommandError::Spec_Violated;
        case CheckErrc::Spec_Violated: return condition == CommandError::Spec_Violated;
        case CheckErrc::Root_Does_Not_Exist: return condition == CommandError::Invalid_Argument;
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
