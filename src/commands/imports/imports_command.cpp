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
        case ImportsErrc::File_Read_Failed: return "Failed to read file";
        case ImportsErrc::Protobuf_Parsing_Failed: return "Protobuf parsing error";
        case ImportsErrc::File_Not_Found: return "File not found";
        case ImportsErrc::Project_Dir_Does_Not_Exist: return "Busrpc project directory does not exist";
        default: return "Unknown error";
        }
    }

    bool equivalent(int code, const std::error_condition& condition) const noexcept override
    {
        switch (static_cast<ImportsErrc>(code)) {
        case ImportsErrc::File_Read_Failed: return condition == CommandError::File_Operation_Failed;
        case ImportsErrc::Protobuf_Parsing_Failed: return condition == CommandError::Protobuf_Parsing_Failed;
        case ImportsErrc::File_Not_Found: return condition == CommandError::Invalid_Argument;
        case ImportsErrc::Project_Dir_Does_Not_Exist: return condition == CommandError::Invalid_Argument;
        default: return false;
        }
    }
};

bool IsSystemFile(std::filesystem::path filePath)
{
    std::filesystem::path sysPath = "google/protobuf";
    filePath.remove_filename();
    return !filePath.empty() &&
           std::search(filePath.begin(), filePath.end(), sysPath.begin(), sysPath.end()) == filePath.begin();
}

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
        if (!IsSystemFile(desc->dependency(i)->name())) {
            FillImportsRecursively(desc->dependency(i), imports);
        }
    }
}
} // namespace

std::error_code ImportsCommand::tryExecuteImpl(std::ostream& out, std::ostream& err) const
{
    ErrorCollector ecol(imports_error_category(), ImportsErrc::Protobuf_Parsing_Failed, err);
    std::set<std::string> imports;
    std::set<std::string> ignored;
    std::filesystem::path projectPath;

    try {
        InitCanonicalPathToExistingDirectory(projectPath, args().projectDir());
    } catch (const std::filesystem::filesystem_error&) { }

    if (projectPath.empty()) {
        ecol.add(ImportsErrc::Project_Dir_Does_Not_Exist, "Project directory '" + args().projectDir() + "' does not exist");
        return ecol.result();
    }

    protobuf::compiler::DiskSourceTree sourceTree;
    sourceTree.MapPath("", projectPath.generic_string());
    protobuf::compiler::Importer importer(&sourceTree, ecol.getProtobufCollector());

    for (const auto& file: args().files()) {
        std::filesystem::path filePath;

        try {
            if (!InitRelativePathToExistingFile(filePath, file, projectPath)) {
                ecol.add(ImportsErrc::File_Not_Found, "File '" + file + "' is not found");
            }
        } catch (const std::filesystem::filesystem_error&) {
            ecol.add(ImportsErrc::File_Read_Failed, "Failed to access file '" + file + "'");
        }

        if (!filePath.empty()) {
            if (args().onlyDeps()) {
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
