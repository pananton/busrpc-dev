#pragma once

#include "commands/command.h"

#include <functional>
#include <string>
#include <system_error>
#include <vector>

/// \dir commands/imports Types and utilites for \c imports command implementation.
/// \file imports_command.h Command \c imports implementation.

namespace CLI {
class App;
}

namespace busrpc {

/// Command-specific error code.
enum class ImportsErrc {
    ///< Failed to parse protobuf file.
    Protobuf_Parsing_Failed = 1,

    ///< Failed to read file for which import tree is built.
    File_Read_Failed = 2,

    ///< File for which import tree should be built is not found.
    File_Not_Found = 3,

    /// Busrpc project directory does not exist or does not represent a valid project directory.
    Invalid_Project_Dir = 4
};

/// Return error category for the \c imports command.
const std::error_category& imports_error_category();

/// Create error code from the \ref ImportsErrc value.
std::error_code make_error_code(ImportsErrc errc);

/// Arguments of the \c imports command.
class ImportsArgs {
public:
    /// Create \c imports command arguments.
    ImportsArgs(std::vector<std::string> files = {},
                std::string projectDir = {},
                std::string protobufRoot = {},
                bool onlyDeps = false):
        files_(std::move(files)),
        projectDir_(std::move(projectDir)),
        protobufRoot_(std::move(protobufRoot)),
        onlyDeps_(onlyDeps)
    { }

    /// Files which imports to output (should be nested in the busrpc project directory).
    const std::vector<std::string>& files() const noexcept { return files_; }

    /// Busrpc project directory.
    /// \note If empty, working directory is assumed.
    const std::string& projectDir() const noexcept { return projectDir_; }

    /// Root directory for protobuf built-in '.proto' files.
    /// \note This is the root directory for such files as 'google/protobuf/descriptor.proto',
    ///       'google/protobuf/any.proto', etc.
    const std::string& protobufRoot() const noexcept { return protobufRoot_; }

    /// Flag indicating whether \ref files themselves should not be outputted.
    bool onlyDeps() const noexcept { return onlyDeps_; }

private:
    std::vector<std::string> files_;
    std::string projectDir_;
    std::string protobufRoot_;
    bool onlyDeps_;
};

/// Output relative paths to the files directly or indirectly imported by the specified file(s).
class ImportsCommand: public Command<CommandId::Imports, ImportsArgs> {
public:
    /// Base type.
    using BaseType = Command<CommandId::Imports, ImportsArgs>;

    /// Create command.
    explicit ImportsCommand(ImportsArgs args) noexcept: BaseType(std::move(args)) { }

protected:
    std::error_code tryExecuteImpl(std::ostream& out, std::ostream& err) const override;
};

/// Define \c imports command line options and set a \a callback to be invoked when \a app encounters the command.
void DefineCommand(CLI::App& app, const std::function<void(ImportsArgs)>& callback);
} // namespace busrpc

namespace std {
template<>
struct is_error_code_enum<busrpc::ImportsErrc>: true_type { };
} // namespace std
