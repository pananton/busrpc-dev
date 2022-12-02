#pragma once

#include "entities/entity.h"

#include <string>
#include <type_traits>

/// \file field.h Structure field.

namespace busrpc {

class Struct;
class Parser;

/// Protobuf type.
enum class ProtobufType {
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
    Map = 16,    ///< protobuf own 'Map' type

    // user-defined
    Enum = 17,   ///< enum
    Message = 18 ///< class
};

/// Return string representation of a protobuf type.
/// \note \c nullptr is returned if \a type is unknown.
constexpr const char* GetProtobufTypeStr(ProtobufType type)
{
    using enum ProtobufType;

    switch (type) {
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

/// Structure field.
class Field: public Entity {
public:
    /// Field type.
    ProtobufType fieldType() const noexcept { return fieldType_; }

    /// Field typename.
    /// \note If field type is \ref ProtobufType::Message or \ref ProtobufType::Enum, then returned name is a
    ///       distinguished name of an entity representing the type (\ref Struct or \ref Enum).
    /// \note For a \ref ProtobufType::Map field this method returns it's type as specified in the protobuf file.
    ///       To access key and value types individually, see \ref MapField.
    const std::string& fieldTypename() const noexcept { return fieldTypename_; }

    /// Field [number](https://developers.google.com/protocol-buffers/docs/proto3#assigning_field_numbers).
    int32_t number() const noexcept { return number_; }

    /// Flag indicating whether field is optional.
    bool isOptional() const noexcept { return flags_ & static_cast<std::underlying_type_t<Flags>>(Flags::Optional); }

    /// Flag indicating whether field is repeated.
    bool isRepeated() const noexcept { return flags_ & static_cast<std::underlying_type_t<Flags>>(Flags::Repeated); }

    /// Flag indicating whether field is observable.
    /// \note Intended for fields, representing method parameters (though can be used for any field).
    bool isObservable() const noexcept
    {
        return flags_ & static_cast<std::underlying_type_t<Flags>>(Flags::Observable);
    }

    /// Flag indicating whether field value is hashed when used as an endpoint component.
    /// \note Intended for fields, representing observable method parameters (though can be used for any field).
    bool isHashed() const noexcept { return flags_ & static_cast<std::underlying_type_t<Flags>>(Flags::Hashed); }

    /// String representation of the default value.
    /// \note Empty string if default value does not exist.
    const std::string& defaultValue() const noexcept { return defaultValue_; }

    /// Structure to which field belongs.
    const Struct* parent() const noexcept;

protected:
    Field(): Entity(EntityType::Field) { }

private:
    friend class Parser;
    enum class Flags { Optional = 1, Repeated = 2, Observable = 4, Hashed = 8 };

    ProtobufType fieldType_ = static_cast<ProtobufType>(0);
    std::string fieldTypename_ = {};
    int32_t number_ = 0;
    std::underlying_type_t<Flags> flags_ = 0;
    std::string defaultValue_ = {};
};

/// Structure field with \c map protobuf type.
class MapField: public Field {
public:
    /// Key type.
    ProtobufType keyType() const noexcept { return keyType_; }

    /// Value type.
    ProtobufType valueType() const noexcept { return valueType_; }

    /// Key type name.
    /// \note If key type is \ref ProtobufType::Message or \ref ProtobufType::Enum, then returned name is a
    ///       distinguished name of an entity representing the type (\ref Struct or \ref Enum).
    const std::string& keyTypename() const noexcept { return keyTypename_; }

    /// Value type name.
    /// \note If value type is \ref ProtobufType::Message or \ref ProtobufType::Enum, then returned name is a
    ///       distinguished name of an entity representing the type (\ref Struct or \ref Enum).
    const std::string& valueTypename() const noexcept { return valueTypename_; }

private:
    ProtobufType keyType_ = static_cast<ProtobufType>(0);
    ProtobufType valueType_ = static_cast<ProtobufType>(0);
    std::string keyTypename_ = {};
    std::string valueTypename_ = {};
};
} // namespace busrpc
