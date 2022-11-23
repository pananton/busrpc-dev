#include "commands/imports/imports_command.h"
#include "commands/error_collector.h"

#include <google/protobuf/compiler/importer.h>

#include <filesystem>
#include <set>
#include <string>

namespace protobuf = google::protobuf;

namespace busrpc {

namespace {
class ImportsErrorCategory: public std::error_category {
public:
    const char* name() const noexcept override { return "imports"; }

    std::string message(int code) const override
    {
        switch (static_cast<ImportsErrc>(code)) {
        case ImportsErrc::File_Read_Error: return "failed to read file";
        case ImportsErrc::Protobuf_Error: return "protobuf parsing error";
        case ImportsErrc::File_Not_Found: return "file not found";
        case ImportsErrc::Non_Existent_Root_Error: return "root directory does not exist";
        default: return "unknown error";
        }
    }

    bool equivalent(int code, const std::error_condition& condition) const noexcept override
    {
        switch (static_cast<ImportsErrc>(code)) {
        case ImportsErrc::File_Read_Error: return condition == CommandError::File_Access_Error;
        case ImportsErrc::Protobuf_Error: return condition == CommandError::Protobuf_Error;
        case ImportsErrc::File_Not_Found: return condition == CommandError::Argument_Error;
        case ImportsErrc::Non_Existent_Root_Error: return condition == CommandError::Argument_Error;
        default: return false;
        }
    }
};

void FillImportsRecursively(const protobuf::FileDescriptor* desc, std::set<std::string>& imports)
{
    if (imports.count(desc->name())) {
        // already processed
        return;
    }

    imports.insert(desc->name());

    for (int i = 0; i < desc->dependency_count(); ++i) {
        FillImportsRecursively(desc->dependency(i), imports);
    }
}
} // namespace

std::error_code ImportsCommand::tryExecuteImpl(std::ostream& out, std::ostream& err) const
{
    ErrorCollector ecol(imports_error_category(), ImportsErrc::Protobuf_Error, err);
    std::set<std::string> imports;

    protobuf::compiler::DiskSourceTree sourceTree;
    sourceTree.MapPath(
        "", args().rootDir.empty() ? std::filesystem::current_path().string().c_str() : args().rootDir.c_str());

    protobuf::compiler::Importer importer(&sourceTree, ecol.getProtobufCollector());

    for (const auto& file: args().files) {
        FillImportsRecursively(importer.Import(file.c_str()), imports);
    }

    for (const auto& file: imports) {
        out << file << std::endl;
    }

    return ecol.result();
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
