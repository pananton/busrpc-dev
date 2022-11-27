#include "app.h"
#include "commands/check/check_command.h"
#include "commands/configure/configure_command.h"
#include "commands/gendoc/gendoc_command.h"
#include "commands/help/help_command.h"
#include "commands/imports/imports_command.h"
#include "commands/version/version_command.h"
#include "configure.h"

#include <CLI/CLI.hpp>

#include <cassert>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace busrpc {

namespace {

struct CheckOptions {
    std::string rootDir = "";
    bool skipDocsChecks = false;
    bool skipStyleChecks = false;
    bool warningAsError = false;
};

struct ConfigureOptions {
    std::string lang = "";
    std::vector<std::string> files = {};
    std::string rootDir = "";
    std::string outputDir = "";

    // group of java options

    std::string javaPackage = "";
    std::string javaOuterClassName = "";
    bool javaMultipleFiles = false;
};

struct GenDocOptions {
    std::string format = "";
    std::string rootDir = "";
    std::string outputDir = "";
};

struct HelpOptions {
    std::string commandName = "";
};

struct ImportsOptions {
    std::vector<std::string> files = {};
    std::string rootDir = "";
    bool onlyDeps = false;
};

template<typename TCommand>
auto CreateInvoker(std::ostream& out, std::ostream& err)
{
    return [&out, &err](typename TCommand::ArgsType args) { TCommand(std::move(args)).execute(out, err); };
}

// Functions to add common options to CLI:App

void AddRootOption(CLI::App& app, std::string& root)
{
    app.add_option("-r,--root", root)
        ->description("Busrpc root directory (the one containing 'api/' and 'services/' subdirectories)")
        ->envname("BUSRPC_ROOT_DIR")
        ->check(CLI::ExistingDirectory);
}

void AddOutputDirOption(CLI::App& app, std::string& outputDir)
{
    app.add_option("-d,--output-dir", outputDir, "Output directory");
}

void AddProtobufFilesPositionalOption(CLI::App& app, std::vector<std::string>& files)
{
    app.add_option("files", files, "Protobuf files");
}

} // namespace

void DefineCommand(CLI::App& app, const std::function<void(CheckArgs)>& callback)
{
    assert(callback);

    auto optsPtr = std::make_shared<CheckOptions>();
    app.description("Check API for conformance to the busrpc specification");

    app.final_callback([callback, optsPtr]() {
        callback(
            {std::move(optsPtr->rootDir), optsPtr->skipDocsChecks, optsPtr->skipStyleChecks, optsPtr->warningAsError});
    });

    AddRootOption(app, optsPtr->rootDir);

    app.add_flag("--skip-docs", optsPtr->skipDocsChecks, "Skip API documentation checks");
    app.add_flag("--skip-style", optsPtr->skipStyleChecks, "Skip API protobuf style checks");
    app.add_flag("-w,--warning-as-error", optsPtr->warningAsError, "Treat warnings as errors");
}

void DefineCommand(CLI::App& app, const std::function<void(ConfigureArgs)>& callback)
{
    assert(callback);

    auto optsPtr = std::make_shared<ConfigureOptions>();
    app.description("Configure protobuf files for the target language");
    app.positionals_at_end(true);

    app.final_callback([callback, optsPtr]() {
        if (optsPtr->lang == GetConfigureLangStr(ConfigureLang::Java)) {
            JavaOptions confOpts{
                std::move(optsPtr->javaPackage), std::move(optsPtr->javaOuterClassName), optsPtr->javaMultipleFiles};

            callback({std::move(confOpts),
                      std::move(optsPtr->files),
                      std::move(optsPtr->rootDir),
                      std::move(optsPtr->outputDir)});
        }
    });

    app.add_option("--lang", optsPtr->lang, "Target language")
        ->required(true)
        ->check(CLI::IsMember(std::set<std::string>{GetConfigureLangStr(ConfigureLang::Java)}));

    AddRootOption(app, optsPtr->rootDir);
    AddOutputDirOption(app, optsPtr->outputDir);
    AddProtobufFilesPositionalOption(app, optsPtr->files);

    // java configuration options

    const char* javaGroup = "Java configuration options";
    auto javaOptsValidator = [&app](const std::string&) -> std::string {
        if (app.get_option("--lang")->as<std::string>() != GetConfigureLangStr(ConfigureLang::Java)) {
            return "Supported only for java language";
        }

        return {};
    };

    app.add_option("--java-package-prefix", optsPtr->javaPackage, "Java package name")
        ->group(javaGroup)
        ->check(javaOptsValidator);
    app.add_option("--java-outer-class", optsPtr->javaPackage, "Java outer class name")
        ->group(javaGroup)
        ->check(javaOptsValidator);
    app.add_flag("--java-multiple-files", optsPtr->javaMultipleFiles, "Use a separate file for each generated class")
        ->group(javaGroup)
        ->check(javaOptsValidator);
}

void DefineCommand(CLI::App& app, const std::function<void(GenDocArgs)>& callback)
{
    assert(callback);

    auto optsPtr = std::make_shared<GenDocOptions>();
    app.description("Generate API documentation");

    app.final_callback([callback, optsPtr]() {
        GenDocFormat format = static_cast<GenDocFormat>(0);

        if (optsPtr->format == GetGenDocFormatStr(GenDocFormat::Json)) {
            format = GenDocFormat::Json;
        }

        assert(format != static_cast<GenDocFormat>(0));

        callback({format, std::move(optsPtr->rootDir), std::move(optsPtr->outputDir)});
    });

    app.add_option("--format", optsPtr->format, "Documentation format")
        ->required(true)
        ->check(CLI::IsMember(std::set<std::string>{GetGenDocFormatStr(GenDocFormat::Json)}));

    AddRootOption(app, optsPtr->rootDir);
    AddOutputDirOption(app, optsPtr->outputDir);
}

void DefineCommand(CLI::App& app, const std::function<void(HelpArgs)>& callback)
{
    assert(callback);

    auto optsPtr = std::make_shared<HelpOptions>();
    app.description("Show help about the command");

    app.final_callback([callback, optsPtr]() {
        std::optional<CommandId> id(GetCommandId(optsPtr->commandName.c_str()));

        if (id == static_cast<CommandId>(0)) {
            id.reset();
        }

        callback({std::move(id)});
    });

    app.add_option("command", optsPtr->commandName, "Name of the command")
        ->check(CLI::IsMember(std::set<std::string>{GetCommandName(CommandId::Check),
                                                    GetCommandName(CommandId::Configure),
                                                    GetCommandName(CommandId::GenDoc),
                                                    GetCommandName(CommandId::Help),
                                                    GetCommandName(CommandId::Imports),
                                                    GetCommandName(CommandId::Version)}));
}

void DefineCommand(CLI::App& app, const std::function<void(ImportsArgs)>& callback)
{
    assert(callback);

    auto optsPtr = std::make_shared<ImportsOptions>();
    app.description("Output relative paths to the files directly or indirectly imported by the specified file(s)");
    app.positionals_at_end(true);

    app.final_callback([callback, optsPtr]() {
        callback({std::move(optsPtr->files), std::move(optsPtr->rootDir), optsPtr->onlyDeps});
    });

    AddRootOption(app, optsPtr->rootDir);
    AddProtobufFilesPositionalOption(app, optsPtr->files);

    app.add_flag("--only-deps",
                 optsPtr->onlyDeps,
                 "Only output paths to the dependencies, do not output paths to the files themselves");
}

void DefineCommand(CLI::App& app, const std::function<void(VersionArgs)>& callback)
{
    assert(callback);

    app.description("Show version information");
    app.final_callback([callback]() { callback({}); });
}

void InitApp(CLI::App& app, std::ostream& out, std::ostream& err)
{
    app.set_version_flag("-v,--version", []() {
        std::ostringstream out;
        VersionCommand({}).execute(out, std::nullopt);
        return out.str();
    });

    DefineCommand(*app.add_subcommand(GetCommandName(CommandId::Check)), CreateInvoker<CheckCommand>(out, err));
    DefineCommand(*app.add_subcommand(GetCommandName(CommandId::Configure)), CreateInvoker<ConfigureCommand>(out, err));
    DefineCommand(*app.add_subcommand(GetCommandName(CommandId::GenDoc)), CreateInvoker<GenDocCommand>(out, err));
    DefineCommand(*app.add_subcommand(GetCommandName(CommandId::Help)), CreateInvoker<HelpCommand>(out, err));
    DefineCommand(*app.add_subcommand(GetCommandName(CommandId::Imports)), CreateInvoker<ImportsCommand>(out, err));
    DefineCommand(*app.add_subcommand(GetCommandName(CommandId::Version)), CreateInvoker<VersionCommand>(out, err));

    app.require_subcommand(0, 1);
}
} // namespace busrpc
