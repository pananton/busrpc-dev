#include "entities/api.h"

#include <cassert>

namespace busrpc {

Api::Api(CompositeEntity* project):
    GeneralCompositeEntity(project, EntityTypeId::Api, Api_Entity_Name, {{Api_Entity_Comment}, {}})
{
    assert(dynamic_cast<Project*>(this->parent()));
    setNestedEntityAddedCallback([this](Entity* entity) { onNestedEntityAdded(entity); });
}

const Project* Api::parent() const noexcept
{
    return static_cast<const Project*>(GeneralCompositeEntity::parent());
}

Project* Api::parent() noexcept
{
    return static_cast<Project*>(GeneralCompositeEntity::parent());
}

Namespace* Api::addNamespace(const std::string& name)
{
    Namespace* ns = addNestedEntity<Namespace>(name);
    namespaces_.insert(ns);
    return ns;
}

void Api::onNestedEntityAdded(Entity* entity)
{
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

        if (enumEntity->parent() == this && enumEntity->name() == Errc_Enumeration_Name) {
            errc_ = enumEntity;
        }
    }
}
} // namespace busrpc
