#pragma once

#include "types.h"

#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

/// \file exception.h Busrpc development tool exceptions.

namespace busrpc {

/// Command error.
class command_error: public std::system_error {
public:
    /// Create command error.
    command_error(CommandId commandId, std::error_code ec):
        std::system_error(ec, GetCommandName(commandId)),
        commandId_(commandId)
    { }

    /// Identifier of the failed command.
    CommandId commandId() const noexcept { return commandId_; }

private:
    CommandId commandId_;
};

/// Entity error.
class entity_error: public std::runtime_error {
public:
    /// Create entity error.
    entity_error(EntityTypeId type, std::string_view dname, std::string_view reason):
        std::runtime_error(MakeErrorDescription(type, dname, reason)),
        type_(type),
        dname_(dname)
    { }

    /// Entity type.
    EntityTypeId type() const noexcept { return type_; }

    /// Entity distinguished name.
    const std::string& dname() const noexcept { return dname_; }

private:
    static std::string MakeErrorDescription(EntityTypeId type, std::string_view dname, std::string_view reason)
    {
        std::string desc = "entity '";

        desc.append(dname);
        desc.append("' (");
        desc.append(GetEntityTypeIdStr(type) ? GetEntityTypeIdStr(type) : "unknown");
        desc.append(") error: ");
        desc.append(reason);

        return desc;
    }

    EntityTypeId type_;
    std::string dname_;
};

/// Name conflicts with existing one.
class name_conflict_error: public entity_error {
public:
    /// Create name clash error.
    name_conflict_error(EntityTypeId entityType,
                        std::string_view entityDistinguishedName,
                        std::string_view conflictingName):
        entity_error(entityType, entityDistinguishedName, MakeErrorDescription(conflictingName)),
        conflictingName_(conflictingName)
    { }

    /// Conflicting name.
    const std::string& conflictingName() const noexcept { return conflictingName_; }

private:
    static std::string MakeErrorDescription(std::string_view conflictingName)
    {
        std::string desc = "name '";
        desc.append(conflictingName);
        desc.append("' conflicts with existing one");
        return desc;
    }

    std::string conflictingName_;
};
} // namespace busrpc
