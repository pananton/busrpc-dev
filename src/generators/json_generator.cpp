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
    obj["enums"] = nullptr;

    for (const auto& enumeration: entity.enums()) {
        if (!doNotAddErrc || enumeration->name() != Errc_Enum_Name) {
            obj["enums"][enumeration->name()] = *enumeration;
        }
    }

    obj["structs"] = nullptr;

    for (const auto& structure: entity.structs()) {
        if (structure->structType() == StructTypeId::General || !onlyGeneral) {
            obj["structs"][structure->name()] = *structure;
        }
    }
}
} // namespace

void JsonGenerator::generate(const Project& project) const
{
    json doc = project;
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

    if (project.implementation()) {
        obj["implementation"] = *project.implementation();
    } else {
        obj["implementation"] = nullptr;
    }

    AddNestedStructsAndEnums(obj, project, true, true);
}

void to_json(json& obj, const Api& api)
{
    AddCommonEntityData(obj, api);
    obj["namespaces"] = nullptr;

    for (const auto& ns: api.namespaces()) {
        obj["namespaces"][ns->name()] = *ns;
    }

    AddNestedStructsAndEnums(obj, api);
}

void to_json(json& obj, const Namespace& ns)
{
    AddCommonEntityData(obj, ns);
    obj["classes"] = nullptr;

    for (const auto& cls: ns.classes()) {
        obj["classes"][cls->name()] = *cls;
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
    obj["methods"] = nullptr;

    for (const auto& method: cls.methods()) {
        obj["methods"][method->name()] = *method;
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

void to_json(json& obj, const Implementation& implementation)
{
    AddCommonEntityData(obj, implementation);
    obj["services"] = nullptr;

    for (const auto& service: implementation.services()) {
        obj["services"][service->name()] = *service;
    }

    AddNestedStructsAndEnums(obj, implementation);
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

    obj["implements"] = nullptr;

    for (const auto& implMethod: service.implementedMethods()) {
        obj["implements"][implMethod.dname()] = implMethod;
    }

    obj["invokes"] = nullptr;

    for (const auto& invkMethod: service.invokedMethods()) {
        obj["invokes"][invkMethod.dname()] = invkMethod;
    }

    AddNestedStructsAndEnums(obj, service);
}

void to_json(json& obj, const ImplementedMethod& implMethod)
{
    obj["dname"] = implMethod.dname();
    obj["docs"] = implMethod.docs();

    if (implMethod.acceptedObjectId()) {
        obj["acceptedObjectId"] = *implMethod.acceptedObjectId();
    }

    for (const auto& acceptedParam: implMethod.acceptedParams()) {
        obj["acceptedParams"][acceptedParam.first] = acceptedParam.second;
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
    obj["fields"] = nullptr;

    for (const auto& field: structure.fields()) {
        obj["fields"][field->name()] = *field;
    }

    AddNestedStructsAndEnums(obj, structure, false);
}

void to_json(json& obj, const Field& field)
{
    AddCommonEntityData(obj, field);

    obj["number"] = field.number();
    obj["fieldTypeName"] = field.fieldTypeName();
    obj["isOptional"] = field.isOptional();
    obj["isRepeated"] = field.isRepeated();
    obj["isObservable"] = field.isObservable();
    obj["isHashed"] = field.isHashed();
    obj["oneofName"] = field.oneofName();
    obj["defaultValue"] = field.defaultValue();
    obj["isMap"] = field.fieldType() == FieldTypeId::Map;

    if (field.fieldType() == FieldTypeId::Map) {
        const auto& mapField = static_cast<const MapField&>(field);

        obj["keyTypeName"] = mapField.keyTypeName();
        obj["valueTypeName"] = mapField.valueTypeName();
    }
}

void to_json(json& obj, const Enum& enumeration)
{
    AddCommonEntityData(obj, enumeration);

    obj["package"] = enumeration.package();
    obj["file"] = enumeration.file().generic_string();
    obj["constants"] = nullptr;

    for (const auto& constant: enumeration.constants()) {
        obj["constants"][constant->name()] = *constant;
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
    obj["commands"] = nullptr;

    for (const auto& command: docs.commands()) {
        obj["commands"][command.first] = command.second;
    }
}
} // namespace busrpc
