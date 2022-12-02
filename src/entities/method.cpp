#include "entities/method.h"
#include "entities/class.h"
#include "entities/struct.h"

#include <cassert>

namespace busrpc {

const Struct* Method::descriptor() const noexcept
{
    auto it = nestedStructs_.find("MethodDesc");
    assert(it != nestedStructs_.end() && it->second->structType() == StructType::Method_Desc);
    return it->second;
}

const Struct* Method::params() const noexcept
{
    auto it = descriptor()->structs().find("Params");

    if (it != descriptor()->structs().end()) {
        assert(it->second->structType() == StructType::Method_Params);
        return it->second;
    } else {
        return nullptr;
    }
}

const Struct* Method::retval() const noexcept
{
    auto it = descriptor()->structs().find("Retval");

    if (it != descriptor()->structs().end()) {
        assert(it->second->structType() == StructType::Method_Retval);
        return it->second;
    } else {
        return nullptr;
    }
}

bool Method::isStatic() const noexcept
{
    return descriptor()->structs().count("Static") != 0;
}

const Class* Method::parent() const noexcept
{
    assert(Entity::parent()->type() == EntityType::Class);
    return static_cast<const Class*>(Entity::parent());
}
} // namespace busrpc
