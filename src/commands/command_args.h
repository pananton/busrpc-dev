#pragma once

#include "exception.h"

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

/// Named command arguments.
/// \note Named arguments specified on the command line as <tt>-key [value]</tt> pairs.
using NamedArgs = std::unordered_map<std::string /* arg key */, std::optional<std::string> /* arv value */>;

/// Positional command argument.
/// \note Positional arguments are bare value, specified on the command line after the last named argument.
using PosArgs = std::vector<std::string>;

/// Command arguments.
class CommandArgs {
public:
    /// Create arguments for command \a cmd.
    CommandArgs(std::string cmd, NamedArgs namedArgs = {}, PosArgs posArgs = {});

    /// Create arguments from the command line.
    /// \throws args_error if command line has incorrect format (see notes)
    /// \note Format of the command line should be the following (note that program name is not included):
    ///         <tt>CMD [-r ROOTDIR] [-[-]named_arg_key [named_arg_value]]... [--] [pos_arg]...</tt>
    /// \note If \a argc is zero, then the effect is the same as invoking default constructor.
    CommandArgs(int argc, const char* argv[]);

    CommandArgs(const CommandArgs&) = default;
    CommandArgs(CommandArgs&&) = default;
    CommandArgs& operator=(const CommandArgs&) = default;
    CommandArgs& operator=(CommandArgs&&) = default;
    ~CommandArgs() = default;

    /// Return command name.
    const std::string& cmd() const noexcept;

    /// Return busrpc root directory.
    /// \note Current directory by default.
    const std::filesystem::path& root() const noexcept;

    /// Return named arguments of the command.
    const NamedArgs& namedArgs() const noexcept;

    /// Return positional arguments of the command.
    const PosArgs& posArgs() const noexcept;

private:

    void handleRootArg(const std::optional<std::string>& value);

    std::string cmd_;
    std::filesystem::path root_;
    NamedArgs namedArgs_;
    PosArgs posArgs_;
};