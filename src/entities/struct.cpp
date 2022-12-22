#include "entities/struct.h"

#include <algorithm>
#include <cassert>

namespace busrpc {

Struct::Struct(CompositeEntity* parent,
               const std::string& name,
               const std::string& filename,
               StructFlags flags,
               EntityDocs docs):
    GeneralCompositeEntity(parent, EntityTypeId::Struct, name, std::move(docs)),
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

    if (this->docs().brief().empty()) {
        setDefaultDescription();
    }

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

bool Struct::isEncodable() const noexcept
{
    for (const auto& field : fields_) {
        if (!IsEncodableField(field->fieldType(), field->flags(), field->oneofName())) {
            return false;
        }
    }

    return true;
}

Field* Struct::addScalarField(const std::string& name,
                              int32_t number,
                              FieldTypeId type,
                              FieldFlags flags,
                              const std::string& oneofName,
                              const std::string& defaultValue,
                              EntityDocs docs)
{
    checkFieldNumberIsFree(name, number);
    Field* field = addNestedEntity<Field>(name, number, type, "", flags, oneofName, defaultValue, std::move(docs));
    fields_.insert(field);
    return field;
}

Field* Struct::addStructField(const std::string& name,
                              int32_t number,
                              const std::string& typeName,
                              FieldFlags flags,
                              const std::string& oneofName,
                              EntityDocs docs)
{
    checkFieldNumberIsFree(name, number);
    Field* field =
        addNestedEntity<Field>(name, number, FieldTypeId::Message, typeName, flags, oneofName, "", std::move(docs));
    fields_.insert(field);
    return field;
}

Field* Struct::addEnumField(const std::string& name,
                            int32_t number,
                            const std::string& typeName,
                            FieldFlags flags,
                            const std::string& oneofName,
                            EntityDocs docs)
{
    checkFieldNumberIsFree(name, number);
    Field* field =
        addNestedEntity<Field>(name, number, FieldTypeId::Enum, typeName, flags, oneofName, "", std::move(docs));
    fields_.insert(field);
    return field;
}

MapField* Struct::addMapField(const std::string& name,
                              int32_t number,
                              FieldTypeId keyType,
                              FieldTypeId valueType,
                              const std::string& valueTypeName,
                              EntityDocs docs)
{
    checkFieldNumberIsFree(name, number);
    MapField* field = addNestedEntity<MapField>(name, number, keyType, valueType, valueTypeName, std::move(docs));
    fields_.insert(field);
    return field;
}

Struct* Struct::addStruct(const std::string& name, StructFlags flags, EntityDocs docs)
{
    return GeneralCompositeEntity::addStruct(name, "", flags, std::move(docs));
}

Enum* Struct::addEnum(const std::string& name, EntityDocs docs)
{
    return GeneralCompositeEntity::addEnum(name, "", std::move(docs));
}

void Struct::setDefaultDescription()
{
    using enum StructTypeId;

    std::vector<std::string> defaultDescription;

    switch (structType_) {
    case Object_Id: defaultDescription.emplace_back(Default_ObjectId_Description); break;
    case Method_Params: defaultDescription.emplace_back(Default_Params_Description); break;
    case Method_Retval: defaultDescription.emplace_back(Default_Retval_Description); break;
    case Static_Marker: defaultDescription.emplace_back(Default_Static_Description); break;
    case Service_Config: defaultDescription.emplace_back(Default_Config_Description); break;
    case Service_Implements: defaultDescription.emplace_back(Default_Implements_Description); break;
    case Service_Invokes: defaultDescription.emplace_back(Default_Invokes_Description); break;
    default: break;
    }

    if (!defaultDescription.empty()) {
        setDocumentation({std::move(defaultDescription), docs().commands()});
    }
}

void Struct::checkFieldNumberIsFree(const std::string& fieldName, int32_t fieldNumber) const
{
    auto result = std::find_if(
        fields_.begin(), fields_.end(), [fieldNumber](const auto& field) { return field->number() == fieldNumber; });

    if (result != fields_.end()) {
        throw name_conflict_error(EntityTypeId::Struct, dname(), fieldName);
    }
}
} // namespace busrpc
