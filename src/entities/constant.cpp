#include "entities/constant.h"
#include "entities/enum.h"

#include <cassert>

namespace busrpc {

const Enum* Constant::parent() const noexcept
{
    assert(Entity::parent()->type() == EntityType::Enum);
    return static_cast<const Enum*>(Entity::parent());
}
} // namespace busrpc
