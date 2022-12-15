#include "entities/field.h"

#include <cassert>

namespace busrpc {

namespace {

std::string GetMapFieldTypeName(FieldTypeId keyType, FieldTypeId valueType, const std::string& valueTypeName)
{
    std::string result = "map<";
    result.append(GetFieldTypeIdStr(keyType));
    result.append(", ");

    if (IsScalarFieldType(valueType)) {
        result.append(GetFieldTypeIdStr(valueType));
    } else {
        result.append(valueTypeName);
    }

    result.append(">");
    return result;
}
} // namespace

Field::Field(CompositeEntity* parent,
             const std::string& name,
             int32_t number,
             FieldTypeId fieldType,
             const std::string& fieldTypeName,
             FieldFlags flags,
             const std::string& oneofName,
             const std::string& defaultValue,
             EntityDocs docs):
    Entity(parent, EntityTypeId::Field, name, std::move(docs)),
    number_(number),
    fieldType_(fieldType),
    fieldTypeName_(fieldTypeName),
    flags_(flags),
    oneofName_(oneofName),
    defaultValue_(defaultValue)
{
    assert(dynamic_cast<Struct*>(this->parent()));

    if (number_ < Min_Field_Number || number_ > Max_Field_Number ||
        (number_ >= Reserved_Field_Number_Range_Start && number_ <= Reserved_Field_Number_Range_End)) {

        throw entity_error(
            EntityTypeId::Struct, this->parent()->dname(), "field '" + this->name() + "' has invalid number");
    }

    if (CheckAll(flags_, FieldFlags::Repeated)) {
        if (CheckAny(flags_, FieldFlags::Optional | FieldFlags::Observable | FieldFlags::Hashed)) {
            throw entity_error(EntityTypeId::Struct,
                               this->parent()->dname(),
                               "field '" + this->name() + "' has mutually-exclusive flags");
        }
    }

    if (!oneofName_.empty() && (CheckAll(flags_, FieldFlags::Repeated) || fieldType_ == FieldTypeId::Map)) {
        throw entity_error(EntityTypeId::Struct,
                           this->parent()->dname(),
                           "field '" + this->name() + "' has type incompatible with oneof");
    }

    if (fieldType_ == FieldTypeId::Map && CheckAll(flags_, FieldFlags::Repeated)) {
        throw entity_error(EntityTypeId::Struct,
                           this->parent()->dname(),
                           "field '" + this->name() + "' of a 'map' type can't be repeated");
    }

    if (fieldType_ == FieldTypeId::Map || fieldType_ == FieldTypeId::Message || fieldType_ == FieldTypeId::Enum) {
        if (fieldTypeName_.empty()) {
            throw entity_error(
                EntityTypeId::Struct, this->parent()->dname(), "field '" + this->name() + "' type is not specified");
        }
    } else {
        fieldTypeName_ = GetFieldTypeIdStr(fieldType_);
    }
}

const Struct* Field::parent() const noexcept
{
    return static_cast<const Struct*>(Entity::parent());
}

Struct* Field::parent() noexcept
{
    return static_cast<Struct*>(Entity::parent());
}

MapField::MapField(CompositeEntity* parent,
                   const std::string& name,
                   int32_t number,
                   FieldTypeId keyType,
                   FieldTypeId valueType,
                   const std::string& valueTypeName,
                   EntityDocs docs):
    Field(parent,
          name,
          number,
          FieldTypeId::Map,
          GetMapFieldTypeName(keyType, valueType, valueTypeName),
          FieldFlags::None,
          "",
          "",
          std::move(docs)),
    keyType_(keyType),
    valueType_(valueType),
    valueTypeName_(valueTypeName)
{
    if (!IsScalarFieldType(keyType_) || keyType_ == FieldTypeId::Float || keyType_ == FieldTypeId::Double ||
        keyType_ == FieldTypeId::Bytes) {

        throw entity_error(
            EntityTypeId::Struct, this->parent()->dname(), "map field '" + this->name() + "' has invalid key type");
    }

    if (valueType_ == FieldTypeId::Map) {
        throw entity_error(
            EntityTypeId::Struct, this->parent()->dname(), "map field '" + this->name() + "' has invalid value type");
    }

    if (valueType_ == FieldTypeId::Message || valueType_ == FieldTypeId::Enum) {
        if (valueTypeName_.empty()) {
            throw entity_error(EntityTypeId::Struct,
                               this->parent()->dname(),
                               "map field '" + this->name() + "' value type is not specified");
        }
    } else {
        valueTypeName_ = GetFieldTypeIdStr(valueType_);
    }
}
} // namespace busrpc
