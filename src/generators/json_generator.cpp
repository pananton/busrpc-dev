#include "generators/json_generator.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace busrpc {

namespace {

void AddCommonEntityData(json& obj, const Entity& entity)
{
    obj["name"] = entity.name();
    obj["dname"] = entity.dname();
    obj["dir"] = entity.dir().generic_string();
    obj["docs"] = entity.docs();
}

void AddNestedStructsAndEnums(json& obj,
                              const GeneralCompositeEntity& entity,
                              bool onlyGeneral = true,
                              bool doNotAddErrc = false)
{
    for (const auto& enumeration: entity.enums()) {
        if (!doNotAddErrc || enumeration->name() != Errc_Enum_Name) {
            obj["enums"][enumeration->name()].push_back(*enumeration);
        }
    }

    for (const auto& structure: entity.structs()) {
        if (structure->structType() == StructTypeId::General || !onlyGeneral) {
            obj["structs"][structure->name()].push_back(*structure);
        }
    }
}
} // namespace

void JsonGenerator::generate(const Project* project) const
{
    json doc = *project;
    out_ << doc;
}

void to_json(json& obj, const Project& project)
{
    AddCommonEntityData(obj, project);

    obj["root"] = project.root();

    if (project.errc()) {
        obj[Errc_Enum_Name] = *project.errc();
    } else {
        obj[Errc_Enum_Name] = nullptr;
    }

    if (project.exception()) {
        obj[GetPredefinedStructName(StructTypeId::Exception)] = *project.exception();
    } else {
        obj[GetPredefinedStructName(StructTypeId::Exception)] = nullptr;
    }

    if (project.callMessage()) {
        obj[GetPredefinedStructName(StructTypeId::Call_Message)] = *project.callMessage();
    } else {
        obj[GetPredefinedStructName(StructTypeId::Call_Message)] = nullptr;
    }

    if (project.resultMessage()) {
        obj[GetPredefinedStructName(StructTypeId::Result_Message)] = *project.resultMessage();
    } else {
        obj[GetPredefinedStructName(StructTypeId::Result_Message)] = nullptr;
    }

    if (project.api()) {
        obj["api"] = *project.api();
    } else {
        obj["api"] = nullptr;
    }

    if (project.services()) {
        obj["services"] = *project.services();
    } else {
        obj["services"] = nullptr;
    }

    AddNestedStructsAndEnums(obj, project, true, true);
}

void to_json(json& obj, const Api& api)
{
    AddCommonEntityData(obj, api);

    for (const auto& ns: api.namespaces()) {
        obj["namespaces"][ns->name()].push_back(*ns);
    }

    AddNestedStructsAndEnums(obj, api);
}

void to_json(json& obj, const Namespace& ns)
{
    AddCommonEntityData(obj, ns);

    for (const auto& cls: ns.classes()) {
        obj["classes"][cls->name()].push_back(*cls);
    }

    AddNestedStructsAndEnums(obj, ns);
}

void to_json(json& obj, const Class& cls)
{
    AddCommonEntityData(obj, cls);

    if (cls.objectId()) {
        obj[GetPredefinedStructName(StructTypeId::Class_Object_Id)] = *cls.objectId();
    } else {
        obj[GetPredefinedStructName(StructTypeId::Class_Object_Id)] = nullptr;
    }

    obj["isStatic"] = cls.isStatic();

    for (const auto& method: cls.methods()) {
        obj["methods"][method->name()].push_back(*method);
    }

    AddNestedStructsAndEnums(obj, cls);
}

void to_json(json& obj, const Method& method)
{
    AddCommonEntityData(obj, method);

    if (method.params()) {
        obj[GetPredefinedStructName(StructTypeId::Method_Params)] = *method.params();
    } else {
        obj[GetPredefinedStructName(StructTypeId::Method_Params)] = nullptr;
    }

    if (method.retval()) {
        obj[GetPredefinedStructName(StructTypeId::Method_Retval)] = *method.retval();
    } else {
        obj[GetPredefinedStructName(StructTypeId::Method_Retval)] = nullptr;
    }

    obj["isStatic"] = method.isStatic();
    obj["isOneway"] = method.isOneway();
    obj["precondition"] = method.precondition();
    obj["postcondition"] = method.postcondition();

    AddNestedStructsAndEnums(obj, method);
}

void to_json(json& obj, const Services& services)
{
    AddCommonEntityData(obj, services);

    for (const auto& service: services.services()) {
        obj["services"][service->name()].push_back(*service);
    }

    AddNestedStructsAndEnums(obj, services);
}

void to_json(json& obj, const Service& service)
{
    AddCommonEntityData(obj, service);

    if (service.config()) {
        obj[GetPredefinedStructName(StructTypeId::Service_Config)] = *service.config();
    } else {
        obj[GetPredefinedStructName(StructTypeId::Service_Config)] = nullptr;
    }

    obj["author"] = service.author();
    obj["email"] = service.email();
    obj["url"] = service.url();

    for (const auto& implMethod: service.implementedMethods()) {
        obj["implements"].push_back(implMethod);
    }

    for (const auto& invkMethod: service.invokedMethods()) {
        obj["invokes"].push_back(invkMethod);
    }

    AddNestedStructsAndEnums(obj, service);
}

void to_json(json& obj, const ImplementedMethod& implMethod)
{
    obj["dname"] = implMethod.dname();
    obj["docs"] = implMethod.docs();

    if (implMethod.acceptedObjectId()) {
        obj["acceptedObjectId"] = *implMethod.acceptedObjectId();
    } else {
        obj["acceptedObjectId"] = nullptr;
    }

    for (const auto& acceptedParam: implMethod.acceptedParams()) {
        obj["acceptedParams"].push_back(std::make_pair(acceptedParam.first, acceptedParam.second));
    }
}

void to_json(json& obj, const InvokedMethod& invkMethod)
{
    obj["dname"] = invkMethod.dname();
    obj["docs"] = invkMethod.docs();
}

void to_json(json& obj, const Struct& structure)
{
    AddCommonEntityData(obj, structure);

    obj["package"] = structure.package();
    obj["file"] = structure.file().generic_string();
    obj["isHashed"] = structure.isHashed();
    obj["isEncodable"] = structure.isEncodable();

    for (const auto& field: structure.fields()) {
        obj["fields"][field->name()].push_back(*field);
    }

    AddNestedStructsAndEnums(obj, structure, false);
}

void to_json(json& obj, const Field& field)
{
    AddCommonEntityData(obj, field);

    obj["number"] = field.number();
    obj["fieldType"] = field.fieldTypeName();
    obj["isMap"] = field.fieldType() == FieldTypeId::Map;
    obj["isOptional"] = field.isOptional();
    obj["isRepeated"] = field.isRepeated();
    obj["isObservable"] = field.isObservable();
    obj["isHashed"] = field.isHashed();
    obj["oneofName"] = field.oneofName();
    obj["defaultValue"] = field.defaultValue();

    if (field.fieldType() == FieldTypeId::Map) {
        const auto& mapField = static_cast<const MapField&>(field);

        obj["keyType"] = mapField.keyTypeName();
        obj["valueType"] = mapField.valueTypeName();
    }
}

void to_json(json& obj, const Enum& enumeration)
{
    AddCommonEntityData(obj, enumeration);

    obj["package"] = enumeration.package();
    obj["file"] = enumeration.file().generic_string();

    for (const auto& constant: enumeration.constants()) {
        obj["constants"][constant->name()].push_back(*constant);
    }
}

void to_json(json& obj, const Constant& constant)
{
    AddCommonEntityData(obj, constant);
    obj["value"] = constant.value();
}

void to_json(json& obj, const EntityDocs& docs)
{
    obj["brief"] = docs.brief();
    obj["description"] = docs.description();

    for (const auto& command: docs.commands()) {
        obj["commands"][command.first] = command.second;
    }
}
} // namespace busrpc
