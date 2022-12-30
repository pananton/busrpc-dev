#include "commands/imports/imports_command.h"
#include "error_collector.h"
#include "utils.h"

#ifdef _MSC_VER
#    pragma warning(push)
#    pragma warning(disable : 4100)
#    pragma warning(disable : 4251)
#else
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wpedantic"
#    pragma GCC diagnostic ignored "-Wconversion"
#    pragma GCC diagnostic ignored "-Wsign-conversion"
#    pragma GCC diagnostic ignored "-Wshadow"
#endif

#include <google/protobuf/compiler/importer.h>

#ifdef _MSC_VER
#    pragma warning(pop)
#else
#    pragma GCC diagnostic pop
#endif

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
        case ImportsErrc::Protobuf_Parsing_Failed: return "Failed to parse protobuf file";
        case ImportsErrc::File_Read_Failed: return "Failed to read file";
        case ImportsErrc::File_Not_Found: return "File not found";
        case ImportsErrc::Invalid_Project_Dir: return "Invalid busrpc project directory";
        default: return "Unknown error";
        }
    }

    bool equivalent(int code, const std::error_condition& condition) const noexcept override
    {
        switch (static_cast<ImportsErrc>(code)) {
        case ImportsErrc::Protobuf_Parsing_Failed: return condition == CommandError::Protobuf_Parsing_Failed;
        case ImportsErrc::File_Read_Failed: return condition == CommandError::File_Operation_Failed;
        case ImportsErrc::File_Not_Found: return condition == CommandError::Invalid_Argument;
        case ImportsErrc::Invalid_Project_Dir: return condition == CommandError::Invalid_Argument;
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
    ErrorCollector ecol(ImportsErrc::Protobuf_Parsing_Failed, SeverityByErrorCodeValue);
    ErrorCollectorGuard ecolGuard(ecol, err);

    std::set<std::string> imports;
    std::set<std::string> ignored;
    std::filesystem::path projectPath;
    std::filesystem::path protobufPath;

    try {
        InitCanonicalPathToExistingDirectory(projectPath, args().projectDir().string());

        if (!args().protobufRoot().empty()) {
            InitCanonicalPathToExistingDirectory(protobufPath, args().protobufRoot().string());
        }
    } catch (const std::filesystem::filesystem_error&) { }

    if (projectPath.empty()) {
        ecol.add(ImportsErrc::Invalid_Project_Dir, std::make_pair("dir", args().projectDir()));
        return ecol.majorError()->code;
    }

    protobuf::compiler::DiskSourceTree sourceTree;
    sourceTree.MapPath("", projectPath.generic_string());

    if (!protobufPath.empty()) {
        sourceTree.MapPath("", protobufPath.generic_string());
    }

#ifndef _WIN32
    sourceTree.MapPath("", "/usr/include");
    sourceTree.MapPath("", "/usr/local/include");
#endif

    protobuf::compiler::Importer importer(&sourceTree, ecol.getProtobufCollector());

    for (const auto& file: args().files()) {
        std::filesystem::path filePath;

        try {
            if (!InitRelativePathToExistingFile(filePath, file, projectPath)) {
                ecol.add(ImportsErrc::File_Not_Found, std::make_pair("file", file));
            }
        } catch (const std::filesystem::filesystem_error&) {
            ecol.add(ImportsErrc::File_Read_Failed, std::make_pair("file", file));
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

    return !ecol ? std::error_code(0, imports_error_category()) : ecol.majorError()->code;
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
