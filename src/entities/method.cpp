#include "entities/method.h"

#include <cassert>

namespace busrpc {

Method::Method(CompositeEntity* cls, const std::string& name): GeneralCompositeEntity(cls, EntityTypeId::Method, name)
{
    assert(dynamic_cast<Class*>(this->parent()));
    setNestedEntityAddedCallback([this](Entity* entity) { onNestedEntityAdded(entity); });
}

const Class* Method::parent() const noexcept
{
    return static_cast<const Class*>(GeneralCompositeEntity::parent());
}

Class* Method::parent() noexcept
{
    return static_cast<Class*>(GeneralCompositeEntity::parent());
}

void Method::onNestedEntityAdded(Entity* entity)
{
    if (entity->type() == EntityTypeId::Struct) {
        auto structEntity = static_cast<Struct*>(entity);

        switch (structEntity->structType()) {
        case StructTypeId::Method_Desc:
            descriptor_ = structEntity;
            setDocumentation(structEntity->docs());
            parseDocCommands();
            break;
        case StructTypeId::Static_Marker: isStatic_ = true; break;
        case StructTypeId::Method_Params: params_ = structEntity; break;
        case StructTypeId::Method_Retval: retval_ = structEntity; break;
        default: break;
        }
    }
}

void Method::parseDocCommands()
{
    auto it = docs().commands().find(doc_cmd::Method_Precondition);

    if (it != docs().commands().end()) {
        assert(!it->second.empty());
        precondition_ = it->second.back();
    }

    it = docs().commands().find(doc_cmd::Method_Postcondition);

    if (it != docs().commands().end()) {
        assert(!it->second.empty());
        postcondition_ = it->second.back();
    }
}
} // namespace busrpc
