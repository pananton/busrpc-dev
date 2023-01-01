#include "commands/gendoc/gendoc_command.h"
#include "generators/json_generator.h"
#include "parser/parser.h"

#include <cassert>
#include <fstream>
#include <string>
#include <system_error>
#include <vector>

namespace busrpc {

namespace {

class GenDocErrorCategory: public std::error_category {
public:
    const char* name() const noexcept override { return "gendoc"; }

    std::string message(int code) const override
    {
        switch (static_cast<GenDocErrc>(code)) {
        case GenDocErrc::Spec_Violated: return "Busrpc specification violated";
        case GenDocErrc::Protobuf_Parsing_Failed: return "Failed to parse protobuf file";
        case GenDocErrc::File_Read_Failed: return "Failed to read source file";
        case GenDocErrc::File_Write_Failed: return "Failed to write generated documentation";
        case GenDocErrc::Invalid_Project_Dir: return "Invalid busrpc project directory";
        default: return "Unknown error";
        }
    }

    bool equivalent(int code, const std::error_condition& condition) const noexcept override
    {
        switch (static_cast<GenDocErrc>(code)) {
        case GenDocErrc::Spec_Violated: return condition == CommandError::Spec_Violated;
        case GenDocErrc::Protobuf_Parsing_Failed: return condition == CommandError::Protobuf_Parsing_Failed;
        case GenDocErrc::File_Read_Failed: return condition == CommandError::File_Operation_Failed;
        case GenDocErrc::File_Write_Failed: return condition == CommandError::File_Operation_Failed;
        case GenDocErrc::Invalid_Project_Dir: return condition == CommandError::Invalid_Argument;
        default: return false;
        }
    }
};
} // namespace

std::error_code GenDocCommand::tryExecuteImpl(std::ostream& out, std::ostream& err) const
{
    std::vector<const std::error_category*> ignoredCategories;
    ignoredCategories.push_back(&spec_warn_category());
    ignoredCategories.push_back(&style_warn_category());

    Parser parser(args().projectDir(), args().protobufRootDir());
    auto [projectPtr, ecol] = parser.parse(std::move(ignoredCategories));
    std::error_code result(0, gendoc_error_category());

    if (ecol) {
        err << ecol;
        ErrorCollector::ErrorInfo majorError = ecol.majorError().value();

        if (majorError.code.category() == parser_error_category()) {
            if (ecol.find(ParserErrc::Invalid_Project_Dir)) {
                result = GenDocErrc::Invalid_Project_Dir;
            } else if (ecol.find(ParserErrc::Read_Failed)) {
                result = GenDocErrc::File_Read_Failed;
            } else {
                result = GenDocErrc::Protobuf_Parsing_Failed;
            }
        } else if (majorError.code.category() == spec_error_category()) {
            result = GenDocErrc::Spec_Violated;
        } else {
            assert(majorError.code.category() == doc_warn_category());
        }
    }

    std::string outputFilename = (args().outputDir() / Json_Doc_File).string();

    if (result != GenDocErrc::Invalid_Project_Dir) {
        std::ofstream outputFile(outputFilename);
        outputFile << std::setw(2);

        if (outputFile.is_open()) {
            JsonGenerator generator(outputFile);
            generator.generate(*projectPtr);
        } else {
            result = GenDocErrc::File_Write_Failed;
        }
    }

    if (!result) {
        out << ("Busrpc project '" + parser.projectDir().string() + "' JSON documentation is written to '" +
                outputFilename + "'")
            << std::endl;
    } else {
        err << ("Failed to build documentation for busrpc project in '" + parser.projectDir().string() + "' directory")
            << std::endl;
    }

    return result;
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
