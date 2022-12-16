#include "entities/field.h"
#include "utils.h"

#include <cassert>

namespace busrpc {

namespace {

bool IsCustomTypeNameValid(const std::string& name)
{
    if (name.empty() || name.back() == '.') {
        return false;
    }

    auto components = SplitString(name, '.');

    for (const auto& component: components) {
        if (!IsValidEntityName(component)) {
            return false;
        }
    }

    return true;
}

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
    fieldTypeName_(IsScalarFieldType(fieldType) ? GetFieldTypeIdStr(fieldType) : fieldTypeName),
    flags_(flags),
    oneofName_(oneofName),
    defaultValue_(defaultValue)
{
    assert(dynamic_cast<Struct*>(this->parent()));

    if (!checkTypeNameIsValid()) {
        throw entity_error(EntityTypeId::Struct,
                           this->parent()->dname(),
                           "field '" + this->name() + "' type name '" + fieldTypeName_ +
                               "' is not a valid protobuf type name");
    }

    if (!checkNumberIsValid()) {
        throw entity_error(EntityTypeId::Struct,
                           this->parent()->dname(),
                           "field '" + this->name() + "' is assigned an invalid/reserved number " +
                               std::to_string(number_));
    }

    if (!checkFlagsAreNotMutuallyExcelusive()) {
        throw entity_error(EntityTypeId::Struct,
                           this->parent()->dname(),
                           "field '" + this->name() + "' options are mutually-exclusive");
    }

    if (!checkFlagsDoNotConflictWithOneof()) {
        throw entity_error(EntityTypeId::Struct,
                           this->parent()->dname(),
                           "field '" + this->name() + "' options are incompatible with 'oneof'");
    }

    if (!checkTypeDoesNotConflictWithOneof()) {
        throw entity_error(EntityTypeId::Struct,
                           this->parent()->dname(),
                           "field '" + this->name() + "' type '" + fieldTypeName_ + "'is incompatible with 'oneof'");
    }

    if (!checkTypeDoesNotConflictWithFlags()) {
        throw entity_error(EntityTypeId::Struct,
                           this->parent()->dname(),
                           "field '" + this->name() + "' type '" + fieldTypeName_ +
                               "'does not allow it to be 'repeated'");
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

bool Field::checkNumberIsValid() const noexcept
{
    if (number_ < Min_Field_Number || number_ > Max_Field_Number ||
        (number_ >= Reserved_Field_Number_Range_Start && number_ <= Reserved_Field_Number_Range_End)) {

        return false;
    }

    return true;
}

bool Field::checkFlagsAreNotMutuallyExcelusive() const noexcept
{
    return !CheckAll(flags_, FieldFlags::Repeated | FieldFlags::Optional);
}

bool Field::checkFlagsDoNotConflictWithOneof() const noexcept
{
    return oneofName_.empty() || !CheckAll(flags_, FieldFlags::Repeated);
}

bool Field::checkTypeDoesNotConflictWithOneof() const noexcept
{
    return oneofName_.empty() || fieldType_ != FieldTypeId::Map;
}

bool Field::checkTypeDoesNotConflictWithFlags() const noexcept
{
    return fieldType_ != FieldTypeId::Map || !CheckAll(flags_, FieldFlags::Repeated);
}

bool Field::checkTypeNameIsValid() const noexcept
{
    if (fieldType_ != FieldTypeId::Message && fieldType_ != FieldTypeId::Enum) {
        return true;
    }

    return IsCustomTypeNameValid(fieldTypeName_);
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
    keyTypeName_(GetFieldTypeIdStr(keyType)),
    valueType_(valueType),
    valueTypeName_(IsScalarFieldType(valueType) ? GetFieldTypeIdStr(valueType) : valueTypeName)
{
    if (!checkKeyTypeIsValid()) {
        throw entity_error(EntityTypeId::Struct,
                           this->parent()->dname(),
                           "map field '" + this->name() + "' key type '" + keyTypeName_ + "' is not allowed");
    }

    if (!checkValueTypeIsValid()) {
        throw entity_error(EntityTypeId::Struct,
                           this->parent()->dname(),
                           "map field '" + this->name() + "' value type '" + valueTypeName_ + "' is not allowed");
    }

    if (!checkValueTypeNameIsValid()) {
        throw entity_error(EntityTypeId::Struct,
                           this->parent()->dname(),
                           "map field '" + this->name() + "' value type name is not a valid protobuf type name");
    }
}

bool MapField::checkKeyTypeIsValid() const noexcept
{
    if (!IsScalarFieldType(keyType_) || keyType_ == FieldTypeId::Float || keyType_ == FieldTypeId::Double ||
        keyType_ == FieldTypeId::Bytes) {

        return false;
    }

    return true;
}

bool MapField::checkValueTypeIsValid() const noexcept
{
    return valueType_ != FieldTypeId::Map;
}

bool MapField::checkValueTypeNameIsValid() const noexcept
{
    if (valueType_ != FieldTypeId::Message && valueType_ != FieldTypeId::Enum) {
        return true;
    }

    return IsCustomTypeNameValid(valueTypeName_);
}
} // namespace busrpc
