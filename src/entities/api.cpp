#include "entities/api.h"
#include "entities/project.h"

namespace busrpc {

const Project* API::parent() const noexcept
{
    return static_cast<const Project*>(Entity::parent());
}
} // namespace busrpc
