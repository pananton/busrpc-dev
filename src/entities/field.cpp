#include "entities/field.h"
#include "entities/struct.h"

namespace busrpc {

const Struct* Field::parent() const noexcept
{
    return static_cast<const Struct*>(Entity::parent());
}
} // namespace busrpc
