#include "commands/check/check_command.h"
#include "parser/parser.h"

#include <cassert>
#include <string>
#include <system_error>
#include <vector>

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

std::error_code CheckCommand::tryExecuteImpl(std::ostream& out, std::ostream& err) const
{
    std::vector<const std::error_category*> ignoredCategories;

    if (args().ignoreSpecWarnings()) {
        ignoredCategories.push_back(&spec_warn_category());
    }

    if (args().ignoreDocWarnings()) {
        ignoredCategories.push_back(&doc_warn_category());
    }

    if (args().ignoreStyleWarnings()) {
        ignoredCategories.push_back(&style_warn_category());
    }

    Parser parser(args().projectDir(), args().protobufRootDir());
    ErrorCollector ecol = parser.parse(std::move(ignoredCategories)).second;
    std::error_code result(0, check_error_category());

    if (ecol) {
        err << ecol;
        auto majorError = ecol.majorError().value();

        if (majorError.code.category() == parser_error_category()) {
            if (ecol.find(ParserErrc::Invalid_Project_Dir)) {
                result = CheckErrc::Invalid_Project_Dir;
            } else if (ecol.find(ParserErrc::Read_Failed)) {
                result = CheckErrc::File_Read_Failed;
            } else {
                result = CheckErrc::Protobuf_Parsing_Failed;
            }
        } else if (majorError.code.category() == spec_error_category()) {
            result = CheckErrc::Spec_Violated;
        } else if (args().warningAsError()) {
            if (majorError.code.category() == spec_warn_category()) {
                result = CheckErrc::Spec_Violated;
            } else if (majorError.code.category() == doc_warn_category()) {
                result = CheckErrc::Doc_Rule_Violated;
            } else {
                assert(majorError.code.category() == style_warn_category());
                result = CheckErrc::Style_Violated;
            }
        }
    }

    if (!result) {
        out << ("Busrpc project in '" + parser.projectDir().string() + "' directory passed all required checks")
            << std::endl;
    } else {
        err << ("Busrpc project in '" + parser.projectDir().string() + "' directory failed some checks") << std::endl;
    }

    return result;
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
