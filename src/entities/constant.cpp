#include "entities/constant.h"

#include <cassert>

namespace busrpc {

Constant::Constant(CompositeEntity* parent, const std::string& name, int32_t value, EntityDocs docs):
    Entity(parent, EntityTypeId::Constant, name, std::move(docs)),
    value_(value)
{
    assert(dynamic_cast<Enum*>(this->parent()));
}

const Enum* Constant::parent() const noexcept
{
    return static_cast<const Enum*>(Entity::parent());
}

Enum* Constant::parent() noexcept
{
    return static_cast<Enum*>(Entity::parent());
}
} // namespace busrpc
