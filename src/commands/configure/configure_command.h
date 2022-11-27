#pragma once

#include "commands/command.h"
#include "commands/configure/lang_options.h"

#include <functional>
#include <string>
#include <system_error>
#include <variant>
#include <vector>

/// \dir commands/configure Types and utilites for \c configure command implementation.
/// \file configure_command.h Command \c configure implementation.

namespace CLI {
class App;
}

namespace busrpc {

/// Command-specific error code.
enum class ConfigureErrc {
    /// Failed to write configured file to the output directory.
    File_Write_Failed = 1,

    /// Failed to read file to be configured.
    File_Read_Failed = 2,

    /// Failed to create output directory.
    Create_Output_Dir_Failed = 3,

    /// Failed to parse protobuf file.
    Protobuf_Parsing_Failed = 4,

    /// File to be configured is not found.
    File_Not_Found = 5,

    /// Busrpc root directory does not exist.
    Root_Does_Not_Exist = 6
};

/// Return error category for the \c configure command.
const std::error_category& configure_error_category();

/// Create error code from the \ref ConfigureErrc value.
std::error_code make_error_code(ConfigureErrc errc);

/// Target language for which files are configured.
enum class ConfigureLang {
    /// Java.
    Java = 1
};

constexpr const char* GetConfigureLangStr(ConfigureLang lang)
{
    switch (lang) {
    case ConfigureLang::Java: return "java";
    default: return nullptr;
    }
}

/// Arguments of the \c configure command.
class ConfigureArgs {
public:
    /// Create \c configure command arguments.
    ConfigureArgs(JavaOptions options,
                  std::vector<std::string> files = {},
                  std::string rootDir = {},
                  std::string outputDir = {}):
        options_(std::move(options)),
        files_(std::move(files)),
        rootDir_(std::move(rootDir)),
        outputDir_(std::move(outputDir_))
    { }

    /// Return target language.
    ConfigureLang lang() const noexcept;

    /// Return configuration options for target language.
    const std::variant<JavaOptions>& options() const noexcept { return options_; }

    /// Return files to configure.
    /// \note Output directory still created (if not already exists), however no files are written to it.
    const std::vector<std::string>& files() const noexcept { return files_; }

    /// Return busrpc root directory (the one containing 'api/' and 'services' subdirectories).
    /// \note If empty, working directory is assumed.
    const std::string& rootDir() const noexcept { return rootDir_; }

    /// Return directory where to write configured files.
    /// \note If empty, '_configured/' subdirectory of the working directory is assumed.
    const std::string& outputDir() const noexcept { return outputDir_; }

private:
    std::variant<JavaOptions> options_;
    std::vector<std::string> files_;
    std::string rootDir_;
    std::string outputDir_;
};

/// Configure protocol files for target language.
/// \note Configuration involves adding some protobuf options intended for the target language to the protocol files.
class ConfigureCommand: public Command<CommandId::Configure, ConfigureArgs> {
public:
    /// Base type.
    using BaseType = Command<CommandId::Configure, ConfigureArgs>;

    /// Create command.
    ConfigureCommand(ConfigureArgs args) noexcept: BaseType(std::move(args)) { }

protected:
    std::error_code tryExecuteImpl(std::ostream& out, std::ostream& err) const override;
};

/// Define \c configure command line options and set a \a callback to be invoked when \a app encounters the command.
void DefineCommand(CLI::App& app, const std::function<void(ConfigureArgs)>& callback);
} // namespace busrpc

namespace std {
template<>
struct is_error_code_enum<busrpc::ConfigureErrc>: true_type { };
} // namespace std
