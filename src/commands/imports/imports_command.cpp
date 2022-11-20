#include "commands/imports/imports_command.h"

#include <string>

namespace busrpc {

namespace {
class ImportsErrorCategory: public std::error_category {
public:
    const char* name() const noexcept override { return "imports"; }

    std::string message(int code) const override
    {
        switch (static_cast<ImportsErrc>(code)) {
        case ImportsErrc::Non_Existent_Root_Error: return "root directory does not exist";
        case ImportsErrc::File_Not_Found: return "file not found";
        case ImportsErrc::Protobuf_Error: return "protobuf parsing error";
        case ImportsErrc::File_Read_Error: return "failed to read file";
        default: return "unknown error";
        }
    }

    bool equivalent(int code, const std::error_condition& condition) const noexcept override
    {
        switch (static_cast<ImportsErrc>(code)) {
        case ImportsErrc::Non_Existent_Root_Error: return condition == CommandError::Argument_Error;
        case ImportsErrc::File_Not_Found: return condition == CommandError::Argument_Error;
        case ImportsErrc::Protobuf_Error: return condition == CommandError::Protobuf_Error;
        case ImportsErrc::File_Read_Error: return condition == CommandError::File_Access_Error;
        default: return false;
        }
    }
};
} // namespace

std::error_code ImportsCommand::tryExecuteImpl(std::ostream&, std::ostream&) const
{
    return {0, imports_error_category()};
}

const std::error_category& imports_error_category()
{
    static const ImportsErrorCategory category;
    return category;
}

std::error_code make_error_code(ImportsErrc e)
{
    return {static_cast<int>(e), imports_error_category()};
}
} // namespace busrpc
