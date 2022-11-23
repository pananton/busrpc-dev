#pragma once

#include "commands/command.h"

#include <functional>
#include <string>
#include <system_error>
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
    File_Write_Error = 1,

    /// Failed to read file to be configured.
    File_Read_Error = 2,

    /// Failed to create output directory.
    Create_Output_Dir_Error = 3,

    /// Failed to parse protobuf file.
    Protobuf_Error = 4,

    /// File to be configured is not found.
    File_Not_Found = 5,

    /// Root directory does not exist.
    Non_Existent_Root_Error = 6
};

/// Return error category for the \c configure command.
const std::error_category& configure_error_category();

/// Create error code from the \ref ConfigureErrc value.
std::error_code make_error_code(ConfigureErrc errc);

/// Target language for which files are configured.
enum class ConfigureLang {
    /// Golang.
    Golang = 1
};

constexpr const char* GetConfigureLangStr(ConfigureLang lang)
{
    switch (lang) {
    case ConfigureLang::Golang: return "go";
    default: return nullptr;
    }
}

/// Arguments of the \c configure command.
struct ConfigureArgs {
    /// Create \c configure command arguments.
    ConfigureArgs(ConfigureLang lang,
                  std::vector<std::string> files = {},
                  std::string rootDir = {},
                  std::string outputDir = {});

    /// Target language (required).
    ConfigureLang lang;

    /// Files to configure (should be specified relatively to the busrpc root directory).
    /// \note Can be empty. In that case, command will create output directory (if does not exist) and then return.
    std::vector<std::string> files = {};

    /// Busrpc root directory.
    /// \note If empty, working directory is assumed.
    std::string rootDir = "";

    /// Output directory.
    /// \note If empty, '_configured/' subdirectory of the working directory is assumed.
    std::string outputDir = "";
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
