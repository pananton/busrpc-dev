#include "entities/class.h"

#include <cassert>

namespace busrpc {

Class::Class(CompositeEntity* ns, const std::string& name): GeneralCompositeEntity(ns, EntityTypeId::Class, name)
{
    assert(dynamic_cast<Namespace*>(this->parent()));
    setNestedEntityAddedCallback([this](Entity* entity) { onNestedEntityAdded(entity); });
}

const Namespace* Class::parent() const noexcept
{
    return static_cast<const Namespace*>(GeneralCompositeEntity::parent());
}

Namespace* Class::parent() noexcept
{
    return static_cast<Namespace*>(GeneralCompositeEntity::parent());
}

Method* Class::addMethod(const std::string& name)
{
    Method* method = addNestedEntity<Method>(name);
    methods_[method->name()] = method;
    return method;
}

void Class::onNestedEntityAdded(Entity* entity)
{
    if (entity->type() == EntityTypeId::Struct) {
        auto structEntity = static_cast<Struct*>(entity);

        switch (structEntity->structType()) {
        case StructTypeId::Class_Desc:
            descriptor_ = structEntity;
            setDocumentation(
                structEntity->description(), structEntity->briefDescription(), structEntity->docCommands());
            break;
        case StructTypeId::Object_Id: objectId_ = structEntity; break;
        default: break;
        }
    }
}
} // namespace busrpc
