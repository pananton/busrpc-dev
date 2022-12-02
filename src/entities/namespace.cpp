#include "entities/namespace.h"
#include "entities/api.h"

namespace busrpc {

const API* Namespace::parent() const noexcept
{
    return static_cast<const API*>(Entity::parent());
}
} // namespace busrpc
