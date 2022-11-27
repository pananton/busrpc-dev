#pragma once

#include <string_view>

/// \file command_id.h Command identifier definition and utilities.

namespace busrpc {

/// Command identifier.
enum class CommandId {
    /// Show help about the command.
    Help = 1,

    /// Output busrpc development tool version.
    Version = 2,

    /// Output files directly or indirectly imported by the specified file(s).
    Imports = 3,

    /// Check API for conformance to the busrpc specification.
    Check = 4,

    /// Generate API documentation.
    GenDoc = 5
};

/// Return command name by \a id.
/// \note Returns \c nullptr if \a id is unknown.
constexpr const char* GetCommandName(CommandId id)
{
    switch (id) {
    case CommandId::Help: return "help";
    case CommandId::Version: return "version";
    case CommandId::Imports: return "imports";
    case CommandId::Check: return "check";
    case CommandId::GenDoc: return "gendoc";
    default: return nullptr;
    }
}

/// Get command identifier by \a name.
/// \note Return 0 if command with name \a name is unknown.
constexpr CommandId GetCommandId(const char* name)
{
    constexpr CommandId unknownId = static_cast<CommandId>(0);

    switch (name[0]) {
    case 'c': return std::string_view(name) == "check" ? CommandId::Check : unknownId;
    case 'g': return std::string_view(name) == "gendoc" ? CommandId::GenDoc : unknownId;
    case 'h': return std::string_view(name) == "help" ? CommandId::Help : unknownId;
    case 'i': return std::string_view(name) == "imports" ? CommandId::Imports : unknownId;
    case 'v': return std::string_view(name) == "version" ? CommandId::Version : unknownId;
    default: return unknownId;
    }
}
} // namespace busrpc
