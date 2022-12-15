#include "entities/api.h"

#include <cassert>

namespace busrpc {

Namespace::Namespace(CompositeEntity* api, const std::string& name):
    GeneralCompositeEntity(api, EntityTypeId::Namespace, name)
{
    assert(dynamic_cast<Api*>(this->parent()));
    setNestedEntityAddedCallback([this](Entity* entity) { onNestedEntityAdded(entity); });
}

const Api* Namespace::parent() const noexcept
{
    return static_cast<const Api*>(GeneralCompositeEntity::parent());
}

Api* Namespace::parent() noexcept
{
    return static_cast<Api*>(GeneralCompositeEntity::parent());
}

Class* Namespace::addClass(const std::string& name)
{
    Class* cls = addNestedEntity<Class>(name);
    classes_[cls->name()] = cls;
    return cls;
}

void Namespace::onNestedEntityAdded(Entity* entity)
{
    if (entity->type() == EntityTypeId::Struct) {
        auto structEntity = static_cast<Struct*>(entity);

        switch (structEntity->structType()) {
        case StructTypeId::Namespace_Desc:
            descriptor_ = structEntity;
            setDocumentation(structEntity->docs());
            break;
        default: break;
        }
    }
}
} // namespace busrpc
