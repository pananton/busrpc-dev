#pragma once

#include "constants.h"

#include <optional>
#include <string_view>
#include <type_traits>

/// \file types.h Common types.

/// Creates neccessary overloads in order to use enum values as binary flags.
#define DEFINE_BITWISE_ENUM(ENUM_TYPE)                                                                 \
    constexpr ENUM_TYPE operator|(ENUM_TYPE lhs, ENUM_TYPE rhs)                                        \
    {                                                                                                  \
        return static_cast<ENUM_TYPE>(static_cast<std::underlying_type_t<ENUM_TYPE>>(lhs) |            \
                                      static_cast<std::underlying_type_t<ENUM_TYPE>>(rhs));            \
    }                                                                                                  \
                                                                                                       \
    constexpr ENUM_TYPE& operator|=(ENUM_TYPE& lhs, ENUM_TYPE rhs)                                     \
    {                                                                                                  \
        return lhs = lhs | rhs;                                                                        \
    }                                                                                                  \
                                                                                                       \
    constexpr ENUM_TYPE operator&(ENUM_TYPE lhs, ENUM_TYPE rhs)                                        \
    {                                                                                                  \
        return static_cast<ENUM_TYPE>(static_cast<std::underlying_type_t<ENUM_TYPE>>(lhs) &            \
                                      static_cast<std::underlying_type_t<ENUM_TYPE>>(rhs));            \
    }                                                                                                  \
                                                                                                       \
    constexpr ENUM_TYPE& operator&=(ENUM_TYPE& lhs, ENUM_TYPE rhs)                                     \
    {                                                                                                  \
        return lhs = lhs & rhs;                                                                        \
    }                                                                                                  \
                                                                                                       \
    constexpr ENUM_TYPE operator~(ENUM_TYPE value)                                                     \
    {                                                                                                  \
        return static_cast<ENUM_TYPE>(~(static_cast<std::underlying_type_t<ENUM_TYPE>>(value)) &       \
                                      static_cast<std::underlying_type_t<ENUM_TYPE>>(ENUM_TYPE::All)); \
    }                                                                                                  \
                                                                                                       \
    constexpr ENUM_TYPE operator^(ENUM_TYPE lhs, ENUM_TYPE rhs)                                        \
    {                                                                                                  \
        return (lhs & ~rhs) | (~lhs & rhs);                                                            \
    }                                                                                                  \
                                                                                                       \
    constexpr ENUM_TYPE& operator^=(ENUM_TYPE& lhs, ENUM_TYPE rhs)                                     \
    {                                                                                                  \
        return lhs = lhs ^ rhs;                                                                        \
    }                                                                                                  \
                                                                                                       \
    constexpr bool CheckAll(ENUM_TYPE value, ENUM_TYPE flags)                                          \
    {                                                                                                  \
        return static_cast<std::underlying_type_t<ENUM_TYPE>>(flags) ==                                \
               (static_cast<std::underlying_type_t<ENUM_TYPE>>(value) &                                \
                static_cast<std::underlying_type_t<ENUM_TYPE>>(flags));                                \
    }                                                                                                  \
                                                                                                       \
    constexpr bool CheckAny(ENUM_TYPE value, ENUM_TYPE flags)                                          \
    {                                                                                                  \
        return static_cast<std::underlying_type_t<ENUM_TYPE>>(flags) == 0 ||                           \
               0 != (static_cast<std::underlying_type_t<ENUM_TYPE>>(value) &                           \
                     static_cast<std::underlying_type_t<ENUM_TYPE>>(flags));                           \
    }

namespace busrpc {

/// Command identifier.
enum class CommandId {
    Help = 1,    ///< Show help about the command.
    Version = 2, ///< Output busrpc development tool version.
    Imports = 3, ///< Output files directly or indirectly imported by the specified file(s).
    Check = 4,   ///< Check API for conformance to the busrpc specification.
    GenDoc = 5   ///< Generate API documentation.
};

/// Get command name.
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

/// Get command identifier.
/// \note Returns \c std::nullopt if \a name is unknown.
constexpr std::optional<CommandId> GetCommandId(std::string_view commandName)
{
    if (commandName.empty()) {
        return std::nullopt;
    }

    switch (commandName[0]) {
    case 'c': return commandName == "check" ? CommandId::Check : std::optional<CommandId>{};
    case 'g': return commandName == "gendoc" ? CommandId::GenDoc : std::optional<CommandId>{};
    case 'h': return commandName == "help" ? CommandId::Help : std::optional<CommandId>{};
    case 'i': return commandName == "imports" ? CommandId::Imports : std::optional<CommandId>{};
    case 'v': return commandName == "version" ? CommandId::Version : std::optional<CommandId>{};
    default: return std::nullopt;
    }
}

/// Get command identifier.
/// \note Returns \c std::nullopt if \a commandName is unknown.
constexpr std::optional<CommandId> GetCommandId(const char* commandName)
{
    return GetCommandId(std::string_view(commandName ? commandName : ""));
}

/// Busrpc entity type identifier.
enum class EntityTypeId {
    Project = 1,             ///< Project (top-level entity).
    Api = 2,                 ///< Project API.
    Services = 3,            ///< Project services.
    Namespace = 4,           ///< Busrpc namespace.
    Class = 5,               ///< Busrpc class.
    Method = 6,              ///< Busrpc method.
    Struct = 7,              ///< Structure (corresponds to protobuf \c message type).
    Field = 8,               ///< Structure field.
    Enum = 9,                ///< Enumeration (corresponds to protobuf \c enum type).
    Constant = 10,           ///< Enumeration constant.
    Service = 11,            ///< Service.
    Implemented_Method = 12, ///< Method implemented by the service.
    Invoked_Method = 13      ///< Method invoked by the service.
};

/// Get string representation of the entity type identifier.
/// \note \c nullptr is returned if \a id is unknown.
constexpr const char* GetEntityTypeIdStr(EntityTypeId id)
{
    switch (id) {
    case EntityTypeId::Project: return "project";
    case EntityTypeId::Api: return "api";
    case EntityTypeId::Services: return "services";
    case EntityTypeId::Namespace: return "namespace";
    case EntityTypeId::Class: return "class";
    case EntityTypeId::Method: return "method";
    case EntityTypeId::Struct: return "struct";
    case EntityTypeId::Field: return "field";
    case EntityTypeId::Enum: return "enum";
    case EntityTypeId::Constant: return "constant";
    case EntityTypeId::Service: return "service";
    case EntityTypeId::Implemented_Method: return "implemented_method";
    case EntityTypeId::Invoked_Method: return "invoked_method";
    default: return nullptr;
    }
}

/// Return \c true if specified \a name is a valid entity name.
/// \note Because busrpc entity names are mapped to a protobuf entities (\c message, \c enum, \c package, etc.), they
///       should satisfy the same constraints. Valid entity name should consist of alphanumerical characters and
///       underscores and should not start with a digit.
constexpr bool IsValidEntityName(std::string_view name)
{
    for (auto sym: name) {
        if (sym == '_' || (sym >= '0' && sym <= '9') || (sym >= 'A' && sym <= 'Z') || (sym >= 'a' && sym <= 'z')) {
            continue;
        }

        return false;
    }

    return !name.empty() && !(name[0] >= '0' && name[0] <= '9');
}

/// Busrpc structure type identifier.
/// \note Structure type identifier determines semantics of the structure.
enum class StructTypeId {
    General = 1,             ///< General structure.
    Namespace_Desc = 2,      ///< Namespace descriptor.
    Class_Desc = 3,          ///< Class descriptor.
    Method_Desc = 4,         ///< Method descriptor.
    Service_Desc = 5,        ///< Service descriptor.
    Call_Message = 6,        ///< Network message representing method call.
    Result_Message = 7,      ///< Network message representing method result.
    Object_Id = 8,           ///< Object identifier.
    Static_Marker = 9,       ///< Static method marker.
    Method_Params = 10,      ///< Method parameters.
    Method_Retval = 11,      ///< Method return value.
    Method_Exception = 12,   ///< Method exception.
    Service_Config = 13,     ///< Service config.
    Service_Implements = 14, ///< Methods implemented by a service expressed as structure fields.
    Service_Invokes = 15     ///< Methods invoked by a service expressed as structure fields.
};

/// Structure entity flags.
enum class StructFlags { None = 0, Hashed = 1, All = 0x1 };
DEFINE_BITWISE_ENUM(StructFlags);

/// Get predefined name of a structure.
/// \note Return \c nullptr if structure does not have a predefined name.
constexpr const char* GetPredefinedStructName(StructTypeId id)
{
    using enum StructTypeId;

    switch (id) {
    case General: return nullptr;
    case Namespace_Desc: return "NamespaceDesc";
    case Class_Desc: return "ClassDesc";
    case Method_Desc: return "MethodDesc";
    case Service_Desc: return "ServiceDesc";
    case Call_Message: return "CallMessage";
    case Result_Message: return "ResultMessage";
    case Object_Id: return "ObjectId";
    case Static_Marker: return "Static";
    case Method_Params: return "Params";
    case Method_Retval: return "Retval";
    case Method_Exception: return "Exception";
    case Service_Config: return "Config";
    case Service_Implements: return "Implements";
    case Service_Invokes: return "Invokes";
    default: return nullptr;
    }
}

/// Get structure type identifier.
/// \note If optional parameter \a parentType is not set, then structure type calculation is based on \a structName
///       only, which means that result may be incorrect if structure is not properly nested. For example, to be
///       considered a namespace descriptor, structure not only should have 'NamespaceDesc' name, but also should be
///       nested in the \ref EntityTypeId::Namespace entity. Parameter \a parentStructType should also be specified if
///       \a parentType is set and equals \ref EntityTypeId::Struct.
constexpr StructTypeId GetStructTypeId(std::string_view structName,
                                       std::optional<EntityTypeId> parentType = std::nullopt,
                                       std::optional<StructTypeId> parentStructType = std::nullopt)
{
    using enum StructTypeId;
    StructTypeId result = General;

    if (structName.empty()) {
        return result;
    }

    switch (structName[0]) {
    case 'C':
        if (structName == "CallMessage") {
            result = Call_Message;
        } else if (structName == "ClassDesc") {
            result = Class_Desc;
        } else if (structName == "Config") {
            result = Service_Config;
        }
        break;
    case 'E':
        if (structName == "Exception") {
            result = Method_Exception;
        }
        break;
    case 'I':
        if (structName == "Implements") {
            result = Service_Implements;
        } else if (structName == "Invokes") {
            result = Service_Invokes;
        }
        break;
    case 'M':
        if (structName == "MethodDesc") {
            result = Method_Desc;
        }
        break;
    case 'N':
        if (structName == "NamespaceDesc") {
            result = Namespace_Desc;
        }
        break;
    case 'O':
        if (structName == "ObjectId") {
            result = Object_Id;
        }
        break;
    case 'P':
        if (structName == "Params") {
            result = Method_Params;
        }
        break;
    case 'R':
        if (structName == "ResultMessage") {
            result = Result_Message;
        } else if (structName == "Retval") {
            result = Method_Retval;
        }
        break;
    case 'S':
        if (structName == "ServiceDesc") {
            result = Service_Desc;
        } else if (structName == "Static") {
            result = Static_Marker;
        }
        break;
    default: break;
    }

    if (result == General || !parentType) {
        return result;
    }

    if (*parentType == EntityTypeId::Struct && !parentStructType) {
        return General;
    }

    switch (result) {
    case Namespace_Desc: return *parentType == EntityTypeId::Namespace ? Namespace_Desc : General;
    case Class_Desc: return *parentType == EntityTypeId::Class ? Class_Desc : General;
    case Method_Desc: return *parentType == EntityTypeId::Method ? Method_Desc : General;
    case Service_Desc: return *parentType == EntityTypeId::Service ? Service_Desc : General;
    case Call_Message: return *parentType == EntityTypeId::Api ? Call_Message : General;
    case Result_Message: return *parentType == EntityTypeId::Api ? Result_Message : General;
    case Object_Id: return *parentType == EntityTypeId::Struct && *parentStructType == Class_Desc ? Object_Id : General;
    case Static_Marker:
        return *parentType == EntityTypeId::Struct && *parentStructType == Method_Desc ? Static_Marker : General;
    case Method_Params:
        return *parentType == EntityTypeId::Struct && *parentStructType == Method_Desc ? Method_Params : General;
    case Method_Retval:
        return *parentType == EntityTypeId::Struct && *parentStructType == Method_Desc ? Method_Retval : General;
    case Method_Exception: return *parentType == EntityTypeId::Api ? Method_Exception : General;
    case Service_Config:
        return *parentType == EntityTypeId::Struct && *parentStructType == Service_Desc ? Service_Config : General;
    case Service_Implements:
        return *parentType == EntityTypeId::Struct && *parentStructType == Service_Desc ? Service_Implements : General;
    case Service_Invokes:
        return *parentType == EntityTypeId::Struct && *parentStructType == Service_Desc ? Service_Invokes : General;
    default: return General;
    }
}

/// Get structure type identifier.
/// \warning It is not necessary that structure with the specified name actually has the type returned (unless the
///          type is \ref StructTypeId::General) because it may not be properly nested. For example, if structure with
///          name 'NamespaceDesc' is not nested in the \ref Namespace entity, then it represents a general structure,
//           not a namespace descriptor.
constexpr StructTypeId GetStructTypeId(const char* structName,
                                       std::optional<EntityTypeId> parentType = std::nullopt,
                                       std::optional<StructTypeId> parentStructType = std::nullopt)
{
    return GetStructTypeId(
        std::string_view(structName ? structName : ""), std::move(parentType), std::move(parentStructType));
}

/// Busrpc structure field type identifier.
enum class FieldTypeId {
    // integer
    Bool = 1,      ///< bool
    Int32 = 2,     ///< int32_t
    Sint32 = 4,    ///< int32_t
    Sfixed32 = 6,  ///< int32_t
    Uint32 = 3,    ///< uint32_t
    Fixed32 = 5,   ///< uint32_t
    Int64 = 7,     ///< int64_t
    Sint64 = 9,    ///< int64_t
    Sfixed64 = 11, ///< int64_t
    Uint64 = 8,    ///< uint64_t
    Fixed64 = 10,  ///< uint64_t

    // floating-point
    Float = 12,  ///< float
    Double = 13, ///< double

    // container
    String = 14, ///< std::string
    Bytes = 15,  ///< std::string
    Map = 16,    ///< 'Map' type provided by the protobuf library

    // user-defined
    Enum = 17,   ///< enum
    Message = 18 ///< class
};

/// Structure field entity flags.
enum class FieldFlags { None = 0, Optional = 1, Repeated = 2, Observable = 4, Hashed = 8, All = 0xf };
DEFINE_BITWISE_ENUM(FieldFlags);

/// Returns \c true if field type is a protobuf scalar type.
/// \note See [here](https://developers.google.com/protocol-buffers/docs/proto3#scalar) for protobuf scalar type
///       definition.
constexpr bool IsScalarFieldType(FieldTypeId id)
{
    using enum FieldTypeId;

    switch (id) {
    case Bool:
    case Int32:
    case Sint32:
    case Sfixed32:
    case Uint32:
    case Fixed32:
    case Int64:
    case Sint64:
    case Sfixed64:
    case Uint64:
    case Fixed64:
    case Float:
    case Double:
    case String:
    case Bytes: return true;
    default: return false;
    }
}

/// Get field type identifier string representation.
/// \note \c nullptr is returned if \a id is unknown.
constexpr const char* GetFieldTypeIdStr(FieldTypeId id)
{
    using enum FieldTypeId;

    switch (id) {
    case Bool: return "bool";
    case Int32: return "int32";
    case Sint32: return "sint32";
    case Sfixed32: return "sfixed32";
    case Uint32: return "uint32";
    case Fixed32: return "fixed32";
    case Int64: return "int64";
    case Sint64: return "sint64";
    case Sfixed64: return "sfixed64";
    case Uint64: return "uint64";
    case Fixed64: return "fixed64";
    case Float: return "float";
    case Double: return "double";
    case String: return "string";
    case Bytes: return "bytes";
    case Map: return "map";
    case Enum: return "enum";
    case Message: return "message";
    default: return nullptr;
    }
}

/// Return \c true if field with the specified \a type, \a flags and \a oneofName can be encoded.
/// \note Whether \ref FieldTypeId::Message field can be encoded depends on the it's nested fields. This function
///       always returns \c false if \a type is \ref FieldTypeId::Message.
/// \note See [specification](https://github.com/pananton/busrpc-spec) for definition of encodable types.
constexpr bool IsEncodableField(FieldTypeId type, FieldFlags flags = FieldFlags::None, std::string_view oneofName = "")
{
    if (!oneofName.empty() || CheckAll(flags, FieldFlags::Repeated)) {
        return false;
    }

    return type == FieldTypeId::Enum ||
           (IsScalarFieldType(type) && type != FieldTypeId::Float && type != FieldTypeId::Double);
}
} // namespace busrpc
