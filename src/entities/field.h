#pragma once

#include "entities/entity.h"
#include "entities/struct.h"
#include "types.h"

#include <cstdint>
#include <string>

/// \file field.h Structure field entity.

namespace busrpc {

class Struct;

/// Structure field entity.
class Field: public Entity {
public:
    /// Structure to which field belongs.
    const Struct* parent() const noexcept;

    /// Structure to which field belongs.
    Struct* parent() noexcept;

    /// Field [number](https://developers.google.com/protocol-buffers/docs/proto3#assigning_field_numbers).
    int32_t number() const noexcept { return number_; }

    /// Identifier of the field type.
    FieldTypeId fieldType() const noexcept { return fieldType_; }

    /// Field type.
    /// \note If field type is \ref FieldTypeId::Message or \ref FieldTypeId::Enum, then returned name is a
    ///       distinguished name of an entity representing the type (\ref Struct or \ref Enum).
    /// \note For a \ref FieldTypeId::Map field this method returns it's type as specified in the protobuf file (for
    ///       example, 'map<int32, MyStruct>'. To access key and value types individually, see \ref MapField.
    const std::string& fieldTypeName() const noexcept { return fieldTypeName_; }

    /// Field flags.
    FieldFlags flags() const noexcept { return flags_; }

    /// Flag indicating whether field is optional.
    bool isOptional() const noexcept { return CheckAll(flags_, FieldFlags::Optional); }

    /// Flag indicating whether field is repeated.
    bool isRepeated() const noexcept { return CheckAll(flags_, FieldFlags::Repeated); }

    /// Flag indicating whether field is observable.
    /// \note Usually used with fields representing method parameters (though can be used for any field).
    bool isObservable() const noexcept { return CheckAll(flags_, FieldFlags::Observable); }

    /// Flag indicating whether field value is hashed when used as an endpoint component.
    /// \note Usually used with fields representing observable method parameters (though can be used for any field).
    bool isHashed() const noexcept { return CheckAll(flags_, FieldFlags::Hashed); }

    /// Name of the protobuf oneof to which field belongs (if any).
    /// \note Empty if field is not part of the oneof.
    const std::string& oneofName() const noexcept { return oneofName_; }

    /// String representation of the default value.
    /// \note Empty string if default value does not exist.
    const std::string& defaultValue() const noexcept { return defaultValue_; }

protected:
    /// Create field entity.
    /// \throws entity_error if field does not represent a valid protobuf \c message field
    Field(CompositeEntity* parent,
          const std::string& name,
          int32_t number,
          FieldTypeId fieldType,
          const std::string& fieldTypeName,
          FieldFlags flags,
          const std::string& oneofName,
          const std::string& defaultValue,
          EntityDocs docs = {});

private:
    friend class CompositeEntity;

    int32_t number_;
    FieldTypeId fieldType_;
    std::string fieldTypeName_;
    FieldFlags flags_ = FieldFlags::None;
    std::string oneofName_;
    std::string defaultValue_;
};

/// Structure field with \c map protobuf type.
class MapField: public Field {
public:
    /// Key type.
    FieldTypeId keyType() const noexcept { return keyType_; }

    /// Value type.
    FieldTypeId valueType() const noexcept { return valueType_; }

    /// Value type name.
    /// \note If value type is \ref FieldTypeId::Message or \ref FieldTypeId::Enum, then returned name is a
    ///       distinguished name of an entity representing the type (\ref Struct or \ref Enum).
    const std::string& valueTypeName() const noexcept { return valueTypeName_; }

protected:
    /// Create field entity for protobuf \c map type.
    MapField(CompositeEntity* parent,
             const std::string& name,
             int32_t number,
             FieldTypeId keyType,
             FieldTypeId valueType,
             const std::string& valueTypeName,
             EntityDocs docs = {});

private:
    friend class CompositeEntity;

    FieldTypeId keyType_;
    FieldTypeId valueType_;
    std::string valueTypeName_;
};
} // namespace busrpc
