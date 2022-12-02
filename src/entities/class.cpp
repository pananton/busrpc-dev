#include "entities/class.h"
#include "entities/namespace.h"
#include "entities/struct.h"

#include <cassert>

namespace busrpc {

const Struct* Class::descriptor() const noexcept
{
    auto it = nestedStructs_.find("ClassDesc");
    assert(it != nestedStructs_.end() && it->second->structType() == StructType::Class_Desc);
    return it->second;
}

const Struct* Class::objectId() const noexcept
{
    auto it = descriptor()->structs().find("ObjectId");

    if (it != descriptor()->structs().end()) {
        assert(it->second->structType() == StructType::Object_Id);
        return it->second;
    } else {
        return nullptr;
    }
}

const Namespace* Class::parent() const noexcept
{
    assert(Entity::parent()->type() == EntityType::Namespace);
    return static_cast<const Namespace*>(Entity::parent());
}
} // namespace busrpc
