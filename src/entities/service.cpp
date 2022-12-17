#include "entities/service.h"
#include "utils.h"

#include <cassert>

namespace busrpc {

namespace {

std::string GetDistinguishedMethodName(const std::string& methodDescDistinguishedName)
{
    // methodDescDistinguishedName should be like <project>.<api>.<namespace>.<class>.<method>.MethodDesc
    auto components = SplitString(methodDescDistinguishedName, '.');

    if (components.size() != 6 || components.back() != GetPredefinedStructName(StructTypeId::Method_Desc)) {
        return std::string{};
    }

    std::string dname = components[0];

    for (size_t i = 1; i < components.size() - 1; ++i) {
        dname.append(".");
        dname.append(components[i]);
    }

    return dname;
}
} // namespace

void ImplementedMethod::parseDocCommands()
{
    auto acceptedValues = docs().commands().find(doc_cmd::Accepted_Value);

    if (acceptedValues == docs().commands().end()) {
        return;
    }

    for (const auto& cmdValue: acceptedValues->second) {
        std::string paramName;
        std::string paramValue;
        auto paramNameEnd = cmdValue.find_first_of(" \t");

        if (paramNameEnd != std::string::npos) {
            paramName = cmdValue.substr(0, paramNameEnd);
            paramValue = TrimString(cmdValue.substr(paramNameEnd + 1));
        } else {
            paramName = cmdValue;
            paramValue = "";
        }

        if (paramName == "@object_id") {
            acceptedObjectId_ = paramValue;
        } else {
            acceptedParams_[paramName] = paramValue;
        }
    }
}

Service::Service(CompositeEntity* services, const std::string& name):
    GeneralCompositeEntity(services, EntityTypeId::Service, name)
{
    assert(dynamic_cast<Services*>(this->parent()));
    setNestedEntityAddedCallback([this](Entity* entity) { onNestedEntityAdded(entity); });
}

const Services* Service::parent() const noexcept
{
    return static_cast<const Services*>(GeneralCompositeEntity::parent());
}

Services* Service::parent() noexcept
{
    return static_cast<Services*>(GeneralCompositeEntity::parent());
}

void Service::onNestedEntityAdded(Entity* entity)
{
    if (entity->type() == EntityTypeId::Struct) {
        auto structEntity = static_cast<Struct*>(entity);

        switch (structEntity->structType()) {
        case StructTypeId::Service_Desc:
            descriptor_ = structEntity;
            setDocumentation(structEntity->docs());
            parseDocCommands();
            break;
        case StructTypeId::Service_Config: config_ = structEntity; break;
        default: break;
        }
    } else if (entity->type() == EntityTypeId::Field) {
        auto fieldEntity = static_cast<Field*>(entity);

        if (fieldEntity->parent()->structType() == StructTypeId::Service_Implements) {
            std::string methodName = GetDistinguishedMethodName(fieldEntity->fieldTypeName());

            if (!methodName.empty()) {
                implementedMethods_.insert(ImplementedMethod(std::move(methodName), fieldEntity->docs()));
            }
        } else if (fieldEntity->parent()->structType() == StructTypeId::Service_Invokes) {
            std::string methodName = GetDistinguishedMethodName(fieldEntity->fieldTypeName());

            if (!methodName.empty()) {
                invokedMethods_.insert(InvokedMethod(std::move(methodName), fieldEntity->docs()));
            }
        }
    }
}

void Service::parseDocCommands()
{
    auto it = docs().commands().find(doc_cmd::Service_Author);

    if (it != docs().commands().end()) {
        assert(!it->second.empty());
        author_ = it->second.back();
    }

    it = docs().commands().find(doc_cmd::Service_Email);

    if (it != docs().commands().end()) {
        assert(!it->second.empty());
        email_ = it->second.back();
    }

    it = docs().commands().find(doc_cmd::Service_Url);

    if (it != docs().commands().end()) {
        assert(!it->second.empty());
        url_ = it->second.back();
    }
}
} // namespace busrpc
