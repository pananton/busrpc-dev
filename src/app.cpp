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

namespace {

struct ConfigureOptions {
    std::string lang = "";
    std::vector<std::string> files = {};
    std::string rootDir = "";
    std::string outputDir = "";
};

struct GenDocOptions {
    std::string format = "";
    std::string rootDir = "";
    std::string outputDir = "";
};

struct HelpOptions {
    std::string commandName = "";
};

template<typename TCommand>
auto CreateInvoker(std::ostream& out, std::ostream& err)
{
    return [&out, &err](typename TCommand::ArgsType args) { TCommand(std::move(args)).execute(out, err); };
}

// Functions to add common options to CLI:App

void AddRootOption(CLI::App& app, std::string& root)
{
    app.add_option("-r,--root", root, "Busrpc root directory (parent of the 'api/' and 'services/' directories)")
        ->envname("BUSRPC_ROOT_DIR")
        ->check(CLI::ExistingDirectory);
}

void AddOutputDirOption(CLI::App& app, std::string& outputDir)
{
    app.add_option("-d,--output-dir", outputDir, "Output directory");
}

void AddProtobufFilesPositionalOption(CLI::App& app, std::vector<std::string>& files)
{
    app.add_option("files", files, "Protobuf files")->check(CLI::ExistingFile);
}

} // namespace

namespace busrpc {

void DefineCommand(CLI::App& app, const std::function<void(CheckArgs)>& callback)
{
    assert(callback);

    auto argsPtr = std::make_shared<CheckArgs>();
    app.description("Check API for conformance to the busrpc specification");
    app.final_callback([callback, argsPtr]() { callback({std::move(*argsPtr)}); });

    AddRootOption(app, argsPtr->rootDir);

    app.add_flag("--skip-docs", argsPtr->skip_docs_checks, "Skip API documentation checks");
    app.add_flag("--skip-style", argsPtr->skip_style_checks, "Skip API protobuf style checks");
    app.add_flag("-w,--warning-as-error", argsPtr->warning_as_error, "Treat warnings as errors");
}

void DefineCommand(CLI::App& app, const std::function<void(ConfigureArgs)>& callback)
{
    assert(callback);

    auto optsPtr = std::make_shared<ConfigureOptions>();
    app.description("Configure protobuf files for target language");
    app.positionals_at_end(true);
    app.final_callback([callback, optsPtr]() {
        ConfigureLang lang = static_cast<ConfigureLang>(0);

        if (optsPtr->lang == GetConfigureLangStr(ConfigureLang::Golang)) {
            lang = ConfigureLang::Golang;
        }

        assert(lang != static_cast<ConfigureLang>(0));

        callback({lang, std::move(optsPtr->files), std::move(optsPtr->rootDir), std::move(optsPtr->outputDir)});
    });

    app.add_option("--lang", optsPtr->lang, "Target language")
        ->required(true)
        ->check(CLI::IsMember(std::set<std::string>{GetConfigureLangStr(ConfigureLang::Golang)}));

    AddRootOption(app, optsPtr->rootDir);
    AddOutputDirOption(app, optsPtr->outputDir);
    AddProtobufFilesPositionalOption(app, optsPtr->files);
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

    auto argsPtr = std::make_shared<ImportsArgs>();
    app.description("Output files directly or indirectly imported by the specified file(s)");
    app.final_callback([callback, argsPtr]() { callback({std::move(*argsPtr)}); });
    app.positionals_at_end(true);

    AddRootOption(app, argsPtr->rootDir);
    AddProtobufFilesPositionalOption(app, argsPtr->files);
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
