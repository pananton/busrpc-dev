#include "entities/service.h"

#include <cassert>

namespace busrpc {

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
        auto calcMethodName = [](const std::string& fieldTypeName) {
            auto pos = fieldTypeName.find_last_of('.');

            if (pos != std::string::npos) {
                return fieldTypeName.substr(0, pos);
            } else {
                return std::string{};
            }
        };

        if (fieldEntity->parent()->structType() == StructTypeId::Service_Implements) {
            auto methodName = calcMethodName(fieldEntity->fieldTypeName());

            if (!methodName.empty()) {
                auto implMethod = addNestedEntity<ImplementedMethod>(methodName, fieldEntity->docs());
                implementedMethods_[implMethod->name()] = implMethod;
            }
        } else if (fieldEntity->parent()->structType() == StructTypeId::Service_Invokes) {
            auto methodName = calcMethodName(fieldEntity->fieldTypeName());

            if (!methodName.empty()) {
                auto invkMethod = addNestedEntity<InvokedMethod>(methodName, fieldEntity->docs());
                invokedMethods_[invkMethod->name()] = invkMethod;
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
