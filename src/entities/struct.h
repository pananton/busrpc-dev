#pragma once

#include "entities/entity.h"
#include "entities/enum.h"
#include "entities/field.h"
#include "types.h"

#include <cstdint>
#include <filesystem>
#include <string>

/// \file struct.h Structure entity.

namespace busrpc {

class Enum;
class Field;
class MapField;

/// Structure entity.
/// \note Represents protobuf \c message type.
class Struct: public GeneralCompositeEntity {
public:
    /// Protobuf package for the corresponding \c message protobuf type.
    const std::string& package() const noexcept { return package_; }

    /// Type of the structure.
    StructTypeId structType() const noexcept { return structType_; }

    /// File for the corresponding \c message protobuf type.
    /// \note Returned value is comprised of \ref Entity::dir value and a filename specified when structure
    ///       was created.
    const std::filesystem::path& file() const noexcept { return file_; }

    /// Field flags.
    StructFlags flags() const noexcept { return flags_; }

    /// Flag indicating whether structure data is hashed when used as a busrpc endpoint component.
    bool isHashed() const noexcept { return CheckAll(flags_, StructFlags::Hashed); }

    /// Structure fields ordered by name.
    const EntityContainer<Field>& fields() const noexcept { return fields_; }

    /// Add field with [scalar](https://developers.google.com/protocol-buffers/docs/proto3#scalar) type.
    /// \throws name_conflict_error if field with the same name is already added
    /// \throws entity_error if field does not represent a valid protobuf \c message field
    Field* addScalarField(const std::string& name,
                          int32_t number,
                          FieldTypeId type,
                          FieldFlags flags = FieldFlags::None,
                          const std::string& oneofName = {},
                          const std::string& defaultValue = {},
                          EntityDocs docs = {});

    /// Add field with custom structure type.
    /// \throws name_conflict_error if field with the same name is already added
    /// \throws entity_error if field does not represent a valid protobuf \c message field
    Field* addStructField(const std::string& name,
                          int32_t number,
                          const std::string& typeName,
                          FieldFlags flags = FieldFlags::None,
                          const std::string& oneofName = {},
                          EntityDocs docs = {});

    /// Add field with enumeration type.
    /// \throws name_conflict_error if field with the same name is already added
    /// \throws entity_error if field does not represent a valid protobuf \c message field
    Field* addEnumField(const std::string& name,
                        int32_t number,
                        const std::string& typeName,
                        FieldFlags flags = FieldFlags::None,
                        const std::string& oneofName = {},
                        EntityDocs docs = {});

    /// Add field with \c map type.
    /// \throws name_conflict_error if field with the same name is already added
    /// \throws entity_error if field does not represent a valid protobuf \c message field
    /// \note Parameter \a valueTypeName is only meaningful if \a valueType is custom user-defined type (protobuf
    ///       \c message or \c enum).
    MapField* addMapField(const std::string& name,
                          int32_t number,
                          FieldTypeId keyType,
                          FieldTypeId valueType,
                          const std::string& valueTypeName = {},
                          EntityDocs docs = {});

    /// Add nested structure.
    /// \throws name_conflict_error if entity with the same name is already added
    Struct* addStruct(const std::string& name,
                      StructFlags flags = StructFlags::None,
                      EntityDocs docs = {});

    /// Add nested enumeration.
    /// \throws name_conflict_error if entity with the same name is already added
    Enum* addEnum(const std::string& name, EntityDocs docs = {});

protected:
    /// Create structure entity.
    Struct(CompositeEntity* parent,
           const std::string& name,
           const std::string& filename,
           StructFlags flags,
           EntityDocs docs);

private:
    friend class CompositeEntity;
    void setDefaultDescription();
    void checkFieldNumberIsFree(const std::string& fieldName, int32_t fieldNumber) const;

    StructTypeId structType_;
    std::string package_;
    std::filesystem::path file_;
    StructFlags flags_ = StructFlags::None;
    EntityContainer<Field> fields_;
};
} // namespace busrpc
