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
            setDocumentation(
                structEntity->description(), structEntity->briefDescription(), structEntity->docCommands());
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
                auto implMethod = addNestedEntity<ImplementedMethod>(methodName,
                                                                     fieldEntity->description(),
                                                                     fieldEntity->briefDescription(),
                                                                     fieldEntity->docCommands());
                implementedMethods_[implMethod->name()] = implMethod;
            }
        } else if (fieldEntity->parent()->structType() == StructTypeId::Service_Invokes) {
            auto methodName = calcMethodName(fieldEntity->fieldTypeName());

            if (!methodName.empty()) {
                auto invkMethod = addNestedEntity<InvokedMethod>(methodName,
                                                                 fieldEntity->description(),
                                                                 fieldEntity->briefDescription(),
                                                                 fieldEntity->docCommands());
                invokedMethods_[invkMethod->name()] = invkMethod;
            }
        }
    }
}

void Service::parseDocCommands()
{
    auto it = docCommands().find(Author_Doc_Command);

    if (it != docCommands().end()) {
        author_ = it->second;
    }

    it = docCommands().find(Email_Doc_Command);

    if (it != docCommands().end()) {
        email_ = it->second;
    }

    it = docCommands().find(Url_Doc_Command);

    if (it != docCommands().end()) {
        url_ = it->second;
    }
}
} // namespace busrpc
