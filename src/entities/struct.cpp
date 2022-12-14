#include "entities/struct.h"

#include <algorithm>
#include <cassert>

namespace busrpc {

Struct::Struct(CompositeEntity* parent,
               const std::string& name,
               const std::string& filename,
               StructFlags flags,
               const std::string& blockComment):
    GeneralCompositeEntity(parent, EntityTypeId::Struct, name, blockComment),
    structType_{},
    package_{},
    file_{},
    flags_(flags),
    fields_{}
{
    assert(this->parent());

    structType_ =
        GetStructTypeId(this->name(),
                        this->parent()->type(),
                        this->parent()->type() == EntityTypeId::Struct
                            ? std::optional<StructTypeId>(static_cast<const Struct*>(this->parent())->structType())
                            : std::nullopt);

    if (this->parent()->type() == EntityTypeId::Struct) {
        package_ = static_cast<Struct*>(this->parent())->package();
        file_ = static_cast<Struct*>(this->parent())->file();
    } else {
        package_ = this->parent()->dname();
        std::filesystem::path file(filename);

        if (!file.has_filename() || file.has_parent_path()) {
            throw entity_error(EntityTypeId::Struct,
                               dname(),
                               "unexpected filename '" + filename +
                                   "' (either invalid or contains directory components)");
        }

        file_ = dir() / file.filename();
    }
}

Field* Struct::addScalarField(const std::string& name,
                              int32_t number,
                              FieldTypeId type,
                              FieldFlags flags,
                              const std::string& oneofName,
                              const std::string& defaultValue,
                              const std::string& blockComment)
{
    checkFieldNumberIsFree(name, number);
    Field* field = addNestedEntity<Field>(name, number, type, "", flags, oneofName, defaultValue, blockComment);
    fields_[field->name()] = field;
    return field;
}

Field* Struct::addStructField(const std::string& name,
                              int32_t number,
                              const std::string& typeName,
                              FieldFlags flags,
                              const std::string& oneofName,
                              const std::string& blockComment)
{
    checkFieldNumberIsFree(name, number);
    Field* field =
        addNestedEntity<Field>(name, number, FieldTypeId::Message, typeName, flags, oneofName, "", blockComment);
    fields_[field->name()] = field;
    return field;
}

Field* Struct::addEnumField(const std::string& name,
                            int32_t number,
                            const std::string& typeName,
                            FieldFlags flags,
                            const std::string& oneofName,
                            const std::string& blockComment)
{
    checkFieldNumberIsFree(name, number);
    Field* field =
        addNestedEntity<Field>(name, number, FieldTypeId::Enum, typeName, flags, oneofName, "", blockComment);
    fields_[field->name()] = field;
    return field;
}

MapField* Struct::addMapField(const std::string& name,
                              int32_t number,
                              FieldTypeId keyType,
                              FieldTypeId valueType,
                              const std::string& valueTypeName,
                              const std::string& blockComment)
{
    checkFieldNumberIsFree(name, number);
    MapField* field = addNestedEntity<MapField>(name, number, keyType, valueType, valueTypeName, blockComment);
    fields_[field->name()] = field;
    return field;
}

Struct* Struct::addStruct(const std::string& name, StructFlags flags, const std::string& blockComment)
{
    return GeneralCompositeEntity::addStruct(name, "", flags, blockComment);
}

Enum* Struct::addEnum(const std::string& name, const std::string& blockComment)
{
    return GeneralCompositeEntity::addEnum(name, "", blockComment);
}

void Struct::checkFieldNumberIsFree(const std::string& fieldName, int32_t fieldNumber) const
{
    auto result = std::find_if(fields_.begin(), fields_.end(), [fieldNumber](const auto& value) {
        return value.second->number() == fieldNumber;
    });

    if (result != fields_.end()) {
        throw name_conflict_error(EntityTypeId::Struct, dname(), fieldName);
    }
}
} // namespace busrpc
