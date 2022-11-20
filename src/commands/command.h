#pragma once

#include "commands/command_id.h"
#include "exception.h"

#include <concepts>
#include <optional>
#include <ostream>
#include <system_error>
#include <type_traits>

/// \dir commands Commands implementation.
/// \file command.h Command base class.

namespace busrpc {

/// Command error.
/// \note This \c enum is registered as an <tt>std::error_condition</tt>. It provides command-independent logical
///       conditions to compare with command-specific detailed <tt>std::error_code</tt>.
/// \note Many commands do not fail fast. When error is encountered, they output diagnostic information to the error
///       stream and continue processing. That means several errors may be encountered during a single invocation of
///       the command and all of them may have distinct error codes, so implementation needs to decide which code to
///       use as the command result. The main idea used by the busrpc development tool is to order command-specific
///       error codes and logical conditions from the \ref CommandError in a way, that errors caused by user (for
///       example, specifying invalid arguments for the command) come before various system errors, and then require
///       command implementations to return code with the minimal value if several errors occurred. Thus, errors that
///       should probably be fixed by user are signalled first.
enum class CommandError {
    /// Invalid command argument.
    Argument_Error = 1,

    /// Logic error.
    Logic_Error = 2,

    /// Failed to parse protobuf file.
    /// \note This error is returned if protobuf file has invalid syntax.
    Protobuf_Error = 3,

    /// Failed to access file or directory (for example, for creating, reading or writing it).
    File_Access_Error = 4,

    /// Internal error.
    Internal_Error = 5
};

/// Return busrpc command error category.
const std::error_category& command_error_category();

/// Create error condition from \ref CommandError.
std::error_condition make_error_condition(CommandError e);

/// Basic command interface.
class CommandBase {
public:
    /// Create command.
    CommandBase(CommandId id) noexcept: id_(id) { }

    /// Default virtual destructor.
    virtual ~CommandBase() = default;

    /// Return command identifier.
    CommandId id() const noexcept { return id_; }

    /// Return command name.
    const char* name() const noexcept { return GetCommandName(id_); }

    /// Execute command using \a out as \c stdout and \a err as stderr.
    /// \throws command_error if some operations of the command did not finish successfully
    /// \note If \a out or \err is not set, corresponding output is discarded as if redirected to \c /dev/null.
    void execute(std::optional<std::reference_wrapper<std::ostream>> out,
                 std::optional<std::reference_wrapper<std::ostream>> err) const;

    /// Execute command using \a out as \c stdout and \a err as stderr.
    /// \note If \a out or \err is not set, corresponding output is discarded as if redirected to \c /dev/null.
    std::error_code tryExecute(std::optional<std::reference_wrapper<std::ostream>> out,
                               std::optional<std::reference_wrapper<std::ostream>> err) const;

protected:
    virtual std::error_code tryExecuteImpl(std::ostream& out, std::ostream& err) const = 0;

private:
    CommandId id_;
};

/// Base class for all commands.
/// \tparam TArgs Type which represents command arguments.
template<CommandId Command_Id, std::move_constructible TArgs>
class Command: public CommandBase {
public:
    /// Command identifier.
    static constexpr CommandId Id = Command_Id;

    /// Command name.
    static constexpr const char* Name = GetCommandName(Command_Id);

    /// Type of command arguments.
    using ArgsType = TArgs;

    /// Create command with arguments \a args.
    Command(ArgsType args): CommandBase(Id), args_(std::move(args)) { }

    /// Return command arguments.
    const TArgs& args() const noexcept { return args_; }

private:
    TArgs args_;
};
} // namespace busrpc

namespace std {
template<>
struct is_error_condition_enum<busrpc::CommandError>: true_type { };
} // namespace std
