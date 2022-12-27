#include "entities/project.h"
#include "utils.h"

#include <cassert>
#include <unordered_set>

namespace busrpc {

namespace {

class SpecErrorCategory: public std::error_category {
public:
    const char* name() const noexcept override { return "spec error"; }

    std::string message(int code) const override
    {
        using enum SpecErrc;

        switch (static_cast<SpecErrc>(code)) {
        case Invalid_Entity: return "Invalid entity";
        case Multiple_Definitions: return "Entity is defined more than once.";
        case Unexpected_Package: return "Entity is defined in unexpected protobuf package";
        case Missing_Builtin: return "Busrpc built-in type could not be found";
        case Nonconforming_Builtin: return "Busrpc built-in type does not conform with specification";
        case Missing_Descriptor: return "Descriptor could not be found";
        case Not_Static_Method: return "Method is not static";
        case Not_Encodable_Type: return "Type is not encodable";
        case Not_Accessible_Type: return "Type is not accessible in the current scope";
        case Unknown_Type: return "Unknown structure field type";
        case Unexpected_Type: return "Unexpected structure field type";
        case Unknown_Method: return "Unknown method";
        default: return "Unknown error";
        }
    }
};

class SpecWarnCategory: public std::error_category {
public:
    const char* name() const noexcept override { return "spec warn"; }

    std::string message(int code) const override
    {
        using enum SpecWarn;

        switch (static_cast<SpecWarn>(code)) {
        case Unexpected_Nested_Entity: return "Entity contains unexpected nested entity";
        default: return "Unknown error";
        }
    }
};

class DocErrorCategory: public std::error_category {
public:
    const char* name() const noexcept override { return "doc error"; }

    std::string message(int code) const override
    {
        using enum DocErrc;

        switch (static_cast<DocErrc>(code)) {
        case Undocumented_Entity: return "Entity is not documented";
        case Unknown_Doc_Command: return "Unknown documentation command";
        default: return "Unknown error";
        }
    }
};

class StyleErrorCategory: public std::error_category {
public:
    const char* name() const noexcept override { return "style error"; }

    std::string message(int code) const override
    {
        using enum StyleErrc;

        switch (static_cast<StyleErrc>(code)) {
        case Invalid_Name_Format: return "Entity name format is invalid";
        default: return "Unknown error";
        }
    }
};

} // namespace

Project::Project(std::filesystem::path root):
    GeneralCompositeEntity(nullptr, EntityTypeId::Project, Project_Entity_Name, {{Project_Entity_Description}, {}}),
    root_(std::move(root))
{
    setNestedEntityAddedCallback([this](Entity* entity) { onNestedEntityAdded(entity); });
    entityDirectory_[name()] = this;
}

Api* Project::addApi()
{
    Api* api = addNestedEntity<Api>();
    api_ = api;
    return api;
}

Services* Project::addServices()
{
    Services* services = addNestedEntity<Services>();
    services_ = services;
    return services;
}

const Entity* Project::find(const std::string& dname) const
{
    std::string prefix = Project_Entity_Name;
    prefix.append(1, '.');
    std::string normalized;

    if (!dname.starts_with(prefix)) {
        normalized = prefix;

        if (!dname.empty() && dname != Project_Entity_Name) {
            normalized.append(dname);
        } else {
            normalized.erase(normalized.size() - 1);
        }
    } else {
        normalized = dname;
    }

    auto it = entityDirectory_.find(normalized);
    return it != entityDirectory_.end() ? it->second : nullptr;
}

ErrorCollector Project::check() const
{
    SeverityOrder orderFunc = [](std::error_code lhs, std::error_code rhs) {
        if (lhs.category() == rhs.category()) {
            return false;
        }

        if (rhs.category() == spec_error_category() ||
            (rhs.category() == doc_error_category() && lhs.category() != spec_error_category()) ||
            (rhs.category() == spec_warn_category() && lhs.category() != spec_error_category() &&
             lhs.category() != doc_error_category())) {
            return true;
        }

        return false;
    };

    ErrorCollector ecol(std::move(orderFunc));
    check(ecol);
    return ecol;
}

void Project::check(ErrorCollector& ecol) const
{
    checkErrc(errc_, ecol);
    checkException(exception_, ecol);
    checkCallMessage(callMessage_, ecol);
    checkResultMessage(resultMessage_, ecol);

    checkNestedStructs(this, ecol);
    checkNestedEnums(this, ecol);

    if (api_) {
        checkApi(api_, ecol);
    }

    if (services_) {
        checkServices(services_, ecol);
    }
}

void Project::onNestedEntityAdded(Entity* entity)
{
    auto isAdded = entityDirectory_.emplace(entity->dname(), entity).second;
    assert(isAdded);

    if (entity->type() == EntityTypeId::Struct) {
        auto structEntity = static_cast<Struct*>(entity);

        switch (structEntity->structType()) {
        case StructTypeId::Call_Message: callMessage_ = structEntity; break;
        case StructTypeId::Result_Message: resultMessage_ = structEntity; break;
        case StructTypeId::Method_Exception: exception_ = structEntity; break;
        default: break;
        }
    } else if (entity->type() == EntityTypeId::Enum) {
        auto enumEntity = static_cast<Enum*>(entity);

        if (enumEntity->parent() == this && enumEntity->name() == Errc_Enum_Name) {
            errc_ = enumEntity;
        }
    }
}

void Project::checkApi(const Api* api, ErrorCollector& ecol) const
{
    checkNestedStructs(api, ecol);
    checkNestedEnums(api, ecol);

    for (const auto& ns: api->namespaces()) {
        checkNamespace(ns, ecol);
    }
}

void Project::checkErrc(const Enum* errc, ErrorCollector& ecol) const
{
    if (!errc) {
        ecol.add(SpecErrc::Missing_Builtin, std::make_pair("builtin", Errc_Enum_Name));
    } else if (errc->file().filename() != Busrpc_Builtin_File) {
        ecol.add(SpecErrc::Missing_Builtin,
                 std::make_pair("builtin", Errc_Enum_Name),
                 "should be defined inside '" + std::string(Busrpc_Builtin_File) + "' files");
    }
}

void Project::checkException(const Struct* exception, ErrorCollector& ecol) const
{
    constexpr const char* typeName = GetPredefinedStructName(StructTypeId::Method_Exception);

    if (!exception) {
        ecol.add(SpecErrc::Missing_Builtin, std::make_pair("builtin", typeName));
        return;
    } else if (exception->file().filename() != Busrpc_Builtin_File) {
        ecol.add(SpecErrc::Missing_Builtin,
                 std::make_pair("builtin", typeName),
                 "should be defined inside '" + std::string(Busrpc_Builtin_File) + "' file");
    } else {
        auto codeIt = exception->fields().find(Exception_Code_Field_Name);

        if (codeIt != exception->fields().end()) {
            auto entityIt = entityDirectory_.find((*codeIt)->fieldTypeName());

            if (entityIt == entityDirectory_.end() || entityIt->second != errc_) {
                ecol.add(SpecErrc::Nonconforming_Builtin,
                         std::make_pair("builtin", typeName),
                         "'" + std::string(Exception_Code_Field_Name) + "' field type should be '" + Errc_Enum_Name +
                             "'");
            } else if (CheckAny((*codeIt)->flags(), FieldFlags::Optional | FieldFlags::Repeated)) {
                ecol.add(SpecErrc::Nonconforming_Builtin,
                         std::make_pair("builtin", typeName),
                         "'" + std::string(Exception_Code_Field_Name) + "' field should not be optional or repeated");
            }
        } else {
            ecol.add(SpecErrc::Nonconforming_Builtin,
                     std::make_pair("builtin", typeName),
                     "'" + std::string(Exception_Code_Field_Name) + "' field does not exist");
        }
    }
}

void Project::checkCallMessage(const Struct* call, ErrorCollector& ecol) const
{
    constexpr const char* typeName = GetPredefinedStructName(StructTypeId::Call_Message);

    if (!call) {
        ecol.add(SpecErrc::Missing_Builtin, std::make_pair("builtin", typeName));
    } else if (call->file().filename() != Busrpc_Builtin_File) {
        ecol.add(SpecErrc::Missing_Builtin,
                 std::make_pair("builtin", typeName),
                 "should be defined inside '" + std::string(Busrpc_Builtin_File) + "' file");
    } else {
        auto oidIt = call->fields().find(Call_Message_Object_Id_Field_Name);
        auto paramsIt = call->fields().find(Call_Message_Params_Field_Name);

        if (oidIt == call->fields().end()) {
            ecol.add(SpecErrc::Nonconforming_Builtin,
                     std::make_pair("builtin", typeName),
                     "'" + std::string(Call_Message_Object_Id_Field_Name) + "' field does not exist");
        } else if ((*oidIt)->fieldType() != FieldTypeId::Bytes) {
            ecol.add(SpecErrc::Nonconforming_Builtin,
                     std::make_pair("builtin", typeName),
                     "'" + std::string(Call_Message_Object_Id_Field_Name) + "' field type should be 'bytes'");
        } else if (!CheckAll((*oidIt)->flags(), FieldFlags::Optional)) {
            ecol.add(SpecErrc::Nonconforming_Builtin,
                     std::make_pair("builtin", typeName),
                     "'" + std::string(Call_Message_Object_Id_Field_Name) + "' field should be optional");
        } else if (CheckAny((*oidIt)->flags(), FieldFlags::Repeated)) {
            ecol.add(SpecErrc::Nonconforming_Builtin,
                     std::make_pair("builtin", typeName),
                     "'" + std::string(Call_Message_Object_Id_Field_Name) + "' field should not be repeated");
        } else if (!(*oidIt)->oneofName().empty()) {
            ecol.add(SpecErrc::Nonconforming_Builtin,
                     std::make_pair("builtin", typeName),
                     "'" + std::string(Call_Message_Object_Id_Field_Name) + "' field should not belong to oneof");
        }

        if (paramsIt == call->fields().end()) {
            ecol.add(SpecErrc::Nonconforming_Builtin,
                     std::make_pair("builtin", typeName),
                     "'" + std::string(Call_Message_Params_Field_Name) + "' field does not exist");
        } else if ((*paramsIt)->fieldType() != FieldTypeId::Bytes) {
            ecol.add(SpecErrc::Nonconforming_Builtin,
                     std::make_pair("builtin", typeName),
                     "'" + std::string(Call_Message_Params_Field_Name) + "' field type should be 'bytes'");
        } else if (!CheckAll((*paramsIt)->flags(), FieldFlags::Optional)) {
            ecol.add(SpecErrc::Nonconforming_Builtin,
                     std::make_pair("builtin", typeName),
                     "'" + std::string(Call_Message_Params_Field_Name) + "' field should be optional");
        } else if (CheckAny((*paramsIt)->flags(), FieldFlags::Repeated)) {
            ecol.add(SpecErrc::Nonconforming_Builtin,
                     std::make_pair("builtin", typeName),
                     "'" + std::string(Call_Message_Params_Field_Name) + "' field should not be repeated");
        } else if (!(*paramsIt)->oneofName().empty()) {
            ecol.add(SpecErrc::Nonconforming_Builtin,
                     std::make_pair("builtin", typeName),
                     "'" + std::string(Call_Message_Params_Field_Name) + "' field should not belong to oneof");
        }

        if (oidIt != call->fields().end() && paramsIt != call->fields().end() && call->fields().size() > 2) {
            ecol.add(SpecErrc::Nonconforming_Builtin,
                     std::make_pair("builtin", typeName),
                     "should contain only '" + std::string(Call_Message_Object_Id_Field_Name) + "' and '" +
                         Call_Message_Params_Field_Name + "' fields");
        }
    }
}

void Project::checkResultMessage(const Struct* result, ErrorCollector& ecol) const
{
    constexpr const char* typeName = GetPredefinedStructName(StructTypeId::Result_Message);

    if (!result) {
        ecol.add(SpecErrc::Missing_Builtin, std::make_pair("builtin", typeName));
    } else if (result->file().filename() != Busrpc_Builtin_File) {
        ecol.add(SpecErrc::Missing_Builtin,
                 std::make_pair("builtin", typeName),
                 "should be defined inside '" + std::string(Busrpc_Builtin_File) + "' file");
    } else {
        auto retvalIt = result->fields().find(Result_Message_Retval_Field_Name);
        auto exceptionIt = result->fields().find(Result_Message_Exception_Field_Name);

        if (retvalIt == result->fields().end()) {
            ecol.add(SpecErrc::Nonconforming_Builtin,
                     std::make_pair("builtin", typeName),
                     "'" + std::string(Result_Message_Retval_Field_Name) + "' field does not exist");
        } else if ((*retvalIt)->fieldType() != FieldTypeId::Bytes) {
            ecol.add(SpecErrc::Nonconforming_Builtin,
                     std::make_pair("builtin", typeName),
                     "'" + std::string(Result_Message_Retval_Field_Name) + "' field type should be 'bytes'");
        }

        if (exceptionIt == result->fields().end()) {
            ecol.add(SpecErrc::Nonconforming_Builtin,
                     std::make_pair("builtin", typeName),
                     "'" + std::string(Result_Message_Exception_Field_Name) + "' field does not exist");
        } else {
            auto entityIt = entityDirectory_.find((*exceptionIt)->fieldTypeName());

            if (entityIt == entityDirectory_.end() || entityIt->second != exception_) {
                ecol.add(SpecErrc::Nonconforming_Builtin,
                         std::make_pair("builtin", typeName),
                         "'" + std::string(Result_Message_Exception_Field_Name) + "' field type should be '" +
                             GetPredefinedStructName(StructTypeId::Method_Exception) + "'");
            }
        }

        if (retvalIt != result->fields().end() && exceptionIt != result->fields().end()) {
            if ((*retvalIt)->oneofName().empty() || (*retvalIt)->oneofName() != (*exceptionIt)->oneofName()) {
                ecol.add(SpecErrc::Nonconforming_Builtin,
                         std::make_pair("builtin", typeName),
                         "fields '" + std::string(Result_Message_Retval_Field_Name) + "' and '" +
                             Result_Message_Exception_Field_Name + "' should belong to the same oneof");
            }

            if (result->fields().size() > 2) {
                ecol.add(SpecErrc::Nonconforming_Builtin,
                         std::make_pair("builtin", typeName),
                         "should contain only '" + std::string(Result_Message_Retval_Field_Name) + "' and '" +
                             Result_Message_Exception_Field_Name + "' fields");
            }
        }
    }
}

void Project::checkNamespace(const Namespace* ns, ErrorCollector& ecol) const
{
    checkNamespaceDesc(ns, ecol);

    if (!IsLowercaseWithUnderscores(ns->name())) {
        ecol.add(StyleErrc::Invalid_Name_Format,
                 std::make_pair(GetEntityTypeIdStr(ns->type()), ns->dname()),
                 "name should consists of lowercase letters, digits and underscores");
    }

    checkNestedStructs(ns, ecol);
    checkNestedEnums(ns, ecol);

    for (const auto& cls: ns->classes()) {
        checkClass(cls, ecol);
    }
}

void Project::checkNamespaceDesc(const Namespace* ns, ErrorCollector& ecol) const
{
    auto desc = ns->descriptor();

    if (!desc) {
        ecol.add(SpecErrc::Missing_Descriptor, std::make_pair(GetEntityTypeIdStr(ns->type()), ns->dname()));
    } else if (desc->file().filename() != Namespace_Desc_File) {
        ecol.add(SpecErrc::Missing_Descriptor,
                 std::make_pair(GetEntityTypeIdStr(ns->type()), ns->dname()),
                 "descriptor should be defined inside '" + std::string(Namespace_Desc_File) + "' file");
    } else if (!desc->fields().empty() || !desc->structs().empty() || !desc->enums().empty()) {
        ecol.add(SpecWarn::Unexpected_Nested_Entity,
                 std::make_pair(GetEntityTypeIdStr(ns->type()), ns->dname()),
                 "deviations from the descriptor format defined in the specification are discouraged");
    }
}

void Project::checkClass(const Class* cls, ErrorCollector& ecol) const
{
    checkClassDesc(cls, ecol);
    checkObjectId(cls, ecol);

    if (!IsLowercaseWithUnderscores(cls->name())) {
        ecol.add(StyleErrc::Invalid_Name_Format,
                 std::make_pair(GetEntityTypeIdStr(cls->type()), cls->dname()),
                 "name should consists of lowercase letters, digits and underscores");
    }

    checkNestedStructs(cls, ecol);
    checkNestedEnums(cls, ecol);

    for (const auto& method: cls->methods()) {
        checkMethod(method, ecol);
    }
}

void Project::checkClassDesc(const Class* cls, ErrorCollector& ecol) const
{
    auto desc = cls->descriptor();

    if (!desc) {
        ecol.add(SpecErrc::Missing_Descriptor, std::make_pair(GetEntityTypeIdStr(cls->type()), cls->dname()));
    } else if (desc->file().filename() != Class_Desc_File) {
        ecol.add(SpecErrc::Missing_Descriptor,
                 std::make_pair(GetEntityTypeIdStr(cls->type()), cls->dname()),
                 "descriptor should be defined inside '" + std::string(Class_Desc_File) + "' file");
    } else {
        bool hasUnexpectedStructs = false;

        for (const auto& structure: desc->structs()) {
            if (structure->structType() != StructTypeId::Object_Id) {
                hasUnexpectedStructs = true;
                break;
            }
        }

        if (!desc->fields().empty() || hasUnexpectedStructs || !desc->enums().empty()) {
            ecol.add(SpecWarn::Unexpected_Nested_Entity,
                     std::make_pair(GetEntityTypeIdStr(cls->type()), cls->dname()),
                     "deviations from the descriptor format defined in the specification are discouraged");
        }
    }
}

void Project::checkObjectId(const Class* cls, ErrorCollector& ecol) const
{
    if (cls->objectId() && !cls->objectId()->isEncodable()) {
        ecol.add(SpecErrc::Not_Encodable_Type,
                 std::make_pair(GetEntityTypeIdStr(cls->type()), cls->dname()),
                 "'" + std::string(GetPredefinedStructName(StructTypeId::Object_Id)) +
                     "' structure should be encodable");
    }
}

void Project::checkMethod(const Method* method, ErrorCollector& ecol) const
{
    checkMethodDesc(method, ecol);

    if (!IsLowercaseWithUnderscores(method->name())) {
        ecol.add(StyleErrc::Invalid_Name_Format,
                 std::make_pair(GetEntityTypeIdStr(method->type()), method->dname()),
                 "name should consists of lowercase letters, digits and underscores");
    }

    checkNestedStructs(method, ecol);
    checkNestedEnums(method, ecol);
}

void Project::checkMethodDesc(const Method* method, ErrorCollector& ecol) const
{
    auto desc = method->descriptor();

    if (!desc) {
        ecol.add(SpecErrc::Missing_Descriptor, std::make_pair(GetEntityTypeIdStr(method->type()), method->dname()));
    } else if (desc->file().filename() != Method_Desc_File) {
        ecol.add(SpecErrc::Missing_Descriptor,
                 std::make_pair(GetEntityTypeIdStr(method->type()), method->dname()),
                 "descriptor should be defined inside '" + std::string(Method_Desc_File) + "' file");
    } else if (method->parent()->descriptor() && method->parent()->isStatic() && !method->isStatic()) {
        ecol.add(SpecErrc::Not_Static_Method,
                 std::make_pair(GetEntityTypeIdStr(method->type()), method->dname()),
                 "static class can contain only static methods");
    } else {
        bool hasUnexpectedStructs = false;

        for (const auto& structure: desc->structs()) {
            if (structure->structType() != StructTypeId::Method_Params &&
                structure->structType() != StructTypeId::Method_Retval &&
                structure->structType() != StructTypeId::Static_Marker) {
                hasUnexpectedStructs = true;
                break;
            }
        }

        if (!desc->fields().empty() || hasUnexpectedStructs || !desc->enums().empty()) {
            ecol.add(SpecWarn::Unexpected_Nested_Entity,
                     std::make_pair(GetEntityTypeIdStr(method->type()), method->dname()),
                     "deviations from the descriptor format defined in the specification are discouraged");
        }
    }
}

void Project::checkServices(const Services* services, ErrorCollector& ecol) const
{
    checkNestedStructs(services, ecol);
    checkNestedEnums(services, ecol);

    for (const auto& service: services->services()) {
        checkService(service, ecol);
    }
}

void Project::checkService(const Service* service, ErrorCollector& ecol) const
{
    checkServiceDesc(service, ecol);
    checkServiceDeps(service, true, ecol);
    checkServiceDeps(service, false, ecol);

    if (!IsLowercaseWithUnderscores(service->name())) {
        ecol.add(StyleErrc::Invalid_Name_Format,
                 std::make_pair(GetEntityTypeIdStr(service->type()), service->dname()),
                 "name should consists of lowercase letters, digits and underscores");
    }

    checkNestedStructs(service, ecol);
    checkNestedEnums(service, ecol);
}

void Project::checkServiceDesc(const Service* service, ErrorCollector& ecol) const
{
    auto desc = service->descriptor();

    if (!desc) {
        ecol.add(SpecErrc::Missing_Descriptor, std::make_pair(GetEntityTypeIdStr(service->type()), service->dname()));
    } else if (desc->file().filename() != Service_Desc_File) {
        ecol.add(SpecErrc::Missing_Descriptor,
                 std::make_pair(GetEntityTypeIdStr(service->type()), service->dname()),
                 "descriptor should be defined inside '" + std::string(Service_Desc_File) + "' file");
    } else {
        bool hasUnexpectedStructs = false;

        for (const auto& structure: desc->structs()) {
            if (structure->structType() != StructTypeId::Service_Config &&
                structure->structType() != StructTypeId::Service_Implements &&
                structure->structType() != StructTypeId::Service_Invokes) {
                hasUnexpectedStructs = true;
                break;
            }
        }

        if (!desc->fields().empty() || hasUnexpectedStructs || !desc->enums().empty()) {
            ecol.add(SpecWarn::Unexpected_Nested_Entity,
                     std::make_pair(GetEntityTypeIdStr(service->type()), service->dname()),
                     "deviations from the descriptor format defined in the specification are discouraged");
        }
    }
}

void Project::checkServiceDeps(const Service* service, bool checkImplemented, ErrorCollector& ecol) const
{
    if (!service->descriptor()) {
        return;
    }

    std::string structName =
        GetPredefinedStructName(checkImplemented ? StructTypeId::Service_Implements : StructTypeId::Service_Invokes);
    auto it = service->descriptor()->structs().find(structName);

    if (it == service->descriptor()->structs().end()) {
        return;
    }

    enum class Error { None, Has_Unknown_Method, Multiple_References };
    Error err = Error::None;
    std::unordered_set<std::string> foundMethods;

    for (const auto& field: (*it)->fields()) {
        if (field->fieldType() != FieldTypeId::Message) {
            err = Error::Has_Unknown_Method;
            break;
        }

        auto typeIt = entityDirectory_.find(field->fieldTypeName());

        if (typeIt == entityDirectory_.end() || typeIt->second->type() != EntityTypeId::Struct) {
            err = Error::Has_Unknown_Method;
            break;
        }

        if (static_cast<const Struct*>(typeIt->second)->structType() != StructTypeId::Method_Desc) {
            err = Error::Has_Unknown_Method;
            break;
        }

        if (!foundMethods.insert(typeIt->second->dname()).second) {
            err = Error::Multiple_References;
            break;
        }
    }

    if (err == Error::Has_Unknown_Method) {
        ecol.add(SpecErrc::Unknown_Method,
                 std::make_pair(GetEntityTypeIdStr(service->type()), service->dname()),
                 "unknown method referenced in '" + structName + "' structure");
    } else if (err == Error::Multiple_References) {
        ecol.add(SpecErrc::Multiple_Definitions,
                 std::make_pair(GetEntityTypeIdStr(service->type()), service->dname()),
                 "same method referenced more than once in '" + structName + "' structure");
    }
}

void Project::checkNestedStructs(const GeneralCompositeEntity* entity, ErrorCollector& ecol) const
{
    for (const auto& structure: entity->structs()) {
        checkStruct(structure, ecol);
    }
}

void Project::checkNestedEnums(const GeneralCompositeEntity* entity, ErrorCollector& ecol) const
{
    for (const auto& enumeration: entity->enums()) {
        checkEnum(enumeration, ecol);
    }
}

void Project::checkStruct(const Struct* structure, ErrorCollector& ecol) const
{
    if (structure->isHashed() && !structure->isEncodable()) {
        ecol.add(SpecErrc::Not_Encodable_Type,
                 std::make_pair(GetEntityTypeIdStr(structure->type()), structure->dname()),
                 "only encodable structures can be hashable");
    }

    std::unordered_set<std::string> allowedDocCommands;
    std::unordered_set<std::string> allowedFieldDocCommands;

    if (structure->structType() == StructTypeId::Method_Desc) {
        allowedDocCommands.insert(doc_cmd::Method_Precondition);
        allowedDocCommands.insert(doc_cmd::Method_Postcondition);
    } else if (structure->structType() == StructTypeId::Service_Desc) {
        allowedDocCommands.insert(doc_cmd::Service_Author);
        allowedDocCommands.insert(doc_cmd::Service_Email);
        allowedDocCommands.insert(doc_cmd::Service_Url);
    } else if (structure->structType() == StructTypeId::Service_Implements) {
        allowedFieldDocCommands.insert(doc_cmd::Accepted_Value);
    }

    checkEntityDocumentation(structure, ecol, allowedDocCommands);

    if (!IsCamelCase(structure->name())) {
        ecol.add(StyleErrc::Invalid_Name_Format,
                 std::make_pair(GetEntityTypeIdStr(structure->type()), structure->dname()),
                 "name should consists of lower and uppercase letters formatted as CamelCase and digits");
    }

    for (const auto& field: structure->fields()) {
        checkField(field, ecol);
    }

    checkNestedStructs(structure, ecol);
    checkNestedEnums(structure, ecol);
}

void Project::checkField(const Field* field, ErrorCollector& ecol) const
{
    // stores structure/enumeration entity used as a field type
    // if current field has 'map' type, then fieldEntityType stores value type if it is custom structure/enumeration
    // special care is taken for field/value type which is structure or enumeration whose typename starts with
    // 'google.' (i.e., type is provided by protobuf library)
    const Entity* nonScalarFieldTypeEntity = nullptr;

    std::string nonScalarFieldTypeName;
    bool isStruct = true;
    bool isFieldTypeValid = true;

    if (!IsScalarFieldType(field->fieldType())) {
        if (field->fieldType() == FieldTypeId::Map) {
            auto mapField = static_cast<const MapField*>(field);

            if (!IsScalarFieldType(mapField->valueType())) {
                nonScalarFieldTypeName = mapField->valueTypeName();
                isStruct = mapField->valueType() == FieldTypeId::Message;
            }
        } else {
            nonScalarFieldTypeName = field->fieldTypeName();
            isStruct = field->fieldType() == FieldTypeId::Message;
        }
    }

    if (!nonScalarFieldTypeName.empty() && !nonScalarFieldTypeName.starts_with("google.")) {
        auto typeIt = entityDirectory_.find(nonScalarFieldTypeName);

        if (typeIt != entityDirectory_.end()) {
            if ((typeIt->second->type() == EntityTypeId::Struct && isStruct) ||
                (typeIt->second->type() == EntityTypeId::Enum && !isStruct)) {

                nonScalarFieldTypeEntity = typeIt->second;
            } else {
                ecol.add(SpecErrc::Unexpected_Type, std::make_pair(GetEntityTypeIdStr(field->type()), field->dname()));
                isFieldTypeValid = false;
            }
        } else {
            ecol.add(SpecErrc::Unknown_Type, std::make_pair(GetEntityTypeIdStr(field->type()), field->dname()));
            isFieldTypeValid = false;
        }
    }

    if (isFieldTypeValid && nonScalarFieldTypeEntity &&
        field->parent()->structType() != StructTypeId::Service_Implements &&
        field->parent()->structType() != StructTypeId::Service_Invokes) {

        if (!field->dir().string().starts_with(nonScalarFieldTypeEntity->dir().string())) {
            ecol.add(SpecErrc::Not_Accessible_Type,
                     std::make_pair(GetEntityTypeIdStr(field->type()), field->dname()),
                     "referenced type '" + nonScalarFieldTypeName + "'");
        }
    }

    if (isFieldTypeValid && (field->isHashed() || field->isObservable())) {
        bool isEncodable = false;

        if (field->fieldType() != FieldTypeId::Message && field->fieldType() != FieldTypeId::Enum) {
            isEncodable = IsEncodableField(field->fieldType(), field->flags(), field->oneofName());
        } else if (nonScalarFieldTypeEntity) {
            isEncodable = field->fieldType() == FieldTypeId::Message
                              ? static_cast<const Struct*>(nonScalarFieldTypeEntity)->isEncodable()
                              : true;
        }

        if (!isEncodable) {
            ecol.add(SpecErrc::Not_Encodable_Type,
                     std::make_pair(GetEntityTypeIdStr(field->type()), field->dname()),
                     "only fields with encodable type can be observable and/or hashable");
        }
    }

    std::unordered_set<std::string> allowedDocCommands;

    if (field->parent()->structType() == StructTypeId::Service_Implements) {
        allowedDocCommands.insert(doc_cmd::Accepted_Value);
    }

    checkEntityDocumentation(field, ecol, allowedDocCommands);

    if (!IsLowercaseWithUnderscores(field->name())) {
        ecol.add(StyleErrc::Invalid_Name_Format,
                 std::make_pair(GetEntityTypeIdStr(field->type()), field->dname()),
                 "name should consists of lowercase letters, digits and underscores");
    }
}

void Project::checkEnum(const Enum* enumeration, ErrorCollector& ecol) const
{
    checkEntityDocumentation(enumeration, ecol);

    if (!IsCamelCase(enumeration->name())) {
        ecol.add(StyleErrc::Invalid_Name_Format,
                 std::make_pair(GetEntityTypeIdStr(enumeration->type()), enumeration->dname()),
                 "name should consists of lower and uppercase letters formatted as CamelCase and digits");
    }

    for (const auto& constant: enumeration->constants()) {
        checkConstant(constant, ecol);
    }
}
void Project::checkConstant(const Constant* constant, ErrorCollector& ecol) const
{
    checkEntityDocumentation(constant, ecol);

    if (!IsUppercaseWithUnderscores(constant->name())) {
        ecol.add(StyleErrc::Invalid_Name_Format,
                 std::make_pair(GetEntityTypeIdStr(constant->type()), constant->dname()),
                 "name should consists of uppercase letters, digits and underscores");
    }
}

void Project::checkEntityDocumentation(const Entity* entity,
                                       ErrorCollector& ecol,
                                       const std::unordered_set<std::string>& allowedDocCommands) const
{
    if (entity->docs().description().empty()) {
        ecol.add(DocErrc::Undocumented_Entity, std::make_pair(GetEntityTypeIdStr(entity->type()), entity->dname()));
    } else {
        for (const auto& cmd: entity->docs().commands()) {
            if (allowedDocCommands.find(cmd.first) == allowedDocCommands.end()) {
                ecol.add(DocErrc::Unknown_Doc_Command,
                         std::make_pair(GetEntityTypeIdStr(entity->type()), entity->dname()),
                         std::make_pair("command", cmd.first));
            }
        }
    }
}

bool Project::isApiEntity(const Entity* entity) const noexcept
{
    for (auto parent = entity->parent(); parent; parent = parent->parent()) {
        if (parent->type() == EntityTypeId::Api) {
            return true;
        }
    }

    return false;
}

const std::error_category& spec_error_category()
{
    static const SpecErrorCategory category;
    return category;
}

std::error_code make_error_code(SpecErrc e)
{
    return {static_cast<int>(e), spec_error_category()};
}

const std::error_category& spec_warn_category()
{
    static const SpecWarnCategory category;
    return category;
}

std::error_code make_error_code(SpecWarn e)
{
    return {static_cast<int>(e), spec_warn_category()};
}

const std::error_category& doc_error_category()
{
    static const DocErrorCategory category;
    return category;
}

std::error_code make_error_code(DocErrc e)
{
    return {static_cast<int>(e), doc_error_category()};
}

const std::error_category& style_error_category()
{
    static const StyleErrorCategory category;
    return category;
}

std::error_code make_error_code(StyleErrc e)
{
    return {static_cast<int>(e), style_error_category()};
}
} // namespace busrpc
