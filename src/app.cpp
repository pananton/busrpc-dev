#include "app.h"
#include "commands/check/check_command.h"
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
    std::string projectDir = {};
    std::string protobufRoot = {};
    bool ignoreSpecWarnings = false;
    bool ignoreDocWarnings = false;
    bool ignoreStyleWarnings = false;
    bool warningAsError = false;
};

struct GenDocOptions {
    std::string format = {};
    std::string projectDir = {};
    std::string outputDir = {};
    std::string protobufRoot = {};
};

struct HelpOptions {
    std::string commandName = {};
};

struct ImportsOptions {
    std::vector<std::string> files = {};
    std::string projectDir = {};
    std::string protobufRoot = {};
    bool onlyDeps = false;
};

template<typename TCommand>
auto CreateInvoker(std::ostream& out, std::ostream& err)
{
    return [outPtr = &out, errPtr = &err](typename TCommand::ArgsType args) {
        TCommand(std::move(args)).execute(outPtr, errPtr);
    };
}

// Functions to add common options to CLI:App

void AddProjectDirOption(CLI::App& app, std::string& projectDir)
{
    app.add_option("-r,--root", projectDir)
        ->description("Busrpc project directory (the one containing 'busrpc.proto' file)")
        ->envname("BUSRPC_PROJECT_DIR")
        ->check(CLI::ExistingDirectory);
}

void AddProtobufRootOption(CLI::App& app, std::string& protobufRoot)
{
    app.add_option("-p,--protobuf-root", protobufRoot)
        ->description("Root directory for protobuf built-in '.proto' files ('google/protobuf/descriptor.proto', etc.)")
        ->envname("BUSRPC_PROTOBUF_ROOT")
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
        callback({std::move(optsPtr->projectDir),
                  std::move(optsPtr->protobufRoot),
                  optsPtr->ignoreSpecWarnings,
                  optsPtr->ignoreDocWarnings,
                  optsPtr->ignoreStyleWarnings,
                  optsPtr->warningAsError});
    });

    AddProjectDirOption(app, optsPtr->projectDir);
    AddProtobufRootOption(app, optsPtr->protobufRoot);

    app.add_flag("--ignore-spec", optsPtr->ignoreSpecWarnings, "Ignore busrpc specification warnings");
    app.add_flag("--ignore-doc", optsPtr->ignoreDocWarnings, "Ignore documentation warnings");
    app.add_flag("--ignore-style", optsPtr->ignoreStyleWarnings, "Ignore style warnings");
    app.add_flag("-w,--warning-as-error", optsPtr->warningAsError, "Treat warnings as errors");
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

        callback(
            {format, std::move(optsPtr->projectDir), std::move(optsPtr->outputDir), std::move(optsPtr->protobufRoot)});
    });

    app.add_option("--format", optsPtr->format, "Documentation format")
        ->default_val(GetGenDocFormatStr(GenDocFormat::Json))
        ->check(CLI::IsMember(std::set<std::string>{GetGenDocFormatStr(GenDocFormat::Json)}));

    AddProjectDirOption(app, optsPtr->projectDir);
    AddOutputDirOption(app, optsPtr->outputDir);
    AddProtobufRootOption(app, optsPtr->protobufRoot);
}

void DefineCommand(CLI::App& app, const std::function<void(HelpArgs)>& callback)
{
    assert(callback);

    auto optsPtr = std::make_shared<HelpOptions>();
    app.description("Show help about the command");

    app.final_callback([callback, optsPtr]() {
        auto id = GetCommandId(optsPtr->commandName.c_str());
        callback({std::move(id)});
    });

    app.add_option("command", optsPtr->commandName, "Name of the command")
        ->check(CLI::IsMember(std::set<std::string>{GetCommandName(CommandId::Check),
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
        callback({std::move(optsPtr->files),
                  std::move(optsPtr->projectDir),
                  std::move(optsPtr->protobufRoot),
                  optsPtr->onlyDeps});
    });

    AddProjectDirOption(app, optsPtr->projectDir);
    AddProtobufRootOption(app, optsPtr->protobufRoot);
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
        std::ostringstream output;
        VersionCommand({}).execute(&output);
        return output.str();
    });

    DefineCommand(*app.add_subcommand(GetCommandName(CommandId::Check)), CreateInvoker<CheckCommand>(out, err));
    DefineCommand(*app.add_subcommand(GetCommandName(CommandId::GenDoc)), CreateInvoker<GenDocCommand>(out, err));
    DefineCommand(*app.add_subcommand(GetCommandName(CommandId::Help)), CreateInvoker<HelpCommand>(out, err));
    DefineCommand(*app.add_subcommand(GetCommandName(CommandId::Imports)), CreateInvoker<ImportsCommand>(out, err));
    DefineCommand(*app.add_subcommand(GetCommandName(CommandId::Version)), CreateInvoker<VersionCommand>(out, err));

    app.require_subcommand(0, 1);
}
} // namespace busrpc
