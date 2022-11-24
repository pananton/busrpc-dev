#include "commands/imports/imports_command.h"
#include "commands/error_collector.h"
#include "utils.h"

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
        case ImportsErrc::File_Read_Failed: return "failed to read file";
        case ImportsErrc::Protobuf_Parsing_Failed: return "protobuf parsing error";
        case ImportsErrc::File_Not_Found: return "file not found";
        case ImportsErrc::Root_Does_Not_Exist: return "busrpc root directory does not exist";
        default: return "unknown error";
        }
    }

    bool equivalent(int code, const std::error_condition& condition) const noexcept override
    {
        switch (static_cast<ImportsErrc>(code)) {
        case ImportsErrc::File_Read_Failed: return condition == CommandError::File_Operation_Failed;
        case ImportsErrc::Protobuf_Parsing_Failed: return condition == CommandError::Protobuf_Parsing_Failed;
        case ImportsErrc::File_Not_Found: return condition == CommandError::Invalid_Argument;
        case ImportsErrc::Root_Does_Not_Exist: return condition == CommandError::Invalid_Argument;
        default: return false;
        }
    }
};

void FillImportsRecursively(const protobuf::FileDescriptor* desc, std::set<std::string>& imports)
{
    if (!desc) {
        return;
    }

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
    ErrorCollector ecol(imports_error_category(), ImportsErrc::Protobuf_Parsing_Failed, err);
    std::set<std::string> imports;
    std::set<std::string> ignored;
    std::filesystem::path rootPath;

    try {
        InitCanonicalPathToExistingDirectory(rootPath, args().rootDir);
    } catch (const std::filesystem::filesystem_error&) { }

    if (rootPath.empty()) {
        ecol.add(ImportsErrc::Root_Does_Not_Exist, "root directory '" + args().rootDir + "' does not exist");
        return ecol.result();
    }

    protobuf::compiler::DiskSourceTree sourceTree;
    sourceTree.MapPath("", rootPath.generic_string());
    protobuf::compiler::Importer importer(&sourceTree, ecol.getProtobufCollector());

    for (const auto& file: args().files) {
        std::filesystem::path filePath;

        try {
            if (!InitRelativePathToExistingFile(filePath, file, rootPath)) {
                ecol.add(ImportsErrc::File_Not_Found, "file '" + file + "' is not found");
            }
        } catch (const std::filesystem::filesystem_error&) {
            ecol.add(ImportsErrc::File_Read_Failed, "failed to access file '" + file + "'");
        }

        if (!filePath.empty()) {
            if (args().only_deps) {
                ignored.insert(filePath.generic_string());
            }

            FillImportsRecursively(importer.Import(filePath.generic_string().c_str()), imports);
        }
    }

    for (const auto& file: imports) {
        if (ignored.count(file) == 0) {
            out << file << std::endl;
        }
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
