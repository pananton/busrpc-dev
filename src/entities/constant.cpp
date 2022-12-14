#include "entities/constant.h"

#include <cassert>

namespace busrpc {

Constant::Constant(CompositeEntity* parent, const std::string& name, int32_t value, const std::string& blockComment):
    Entity(parent, EntityTypeId::Constant, name, blockComment),
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
