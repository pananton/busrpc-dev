#include "entities/invoked_method.h"

#include <cassert>

namespace busrpc {

InvokedMethod::InvokedMethod(CompositeEntity* service, const std::string& name, EntityDocs docs):
    Entity(service, EntityTypeId::Implemented_Method, name, std::move(docs))
{
    assert(dynamic_cast<Service*>(this->parent()));
}

const Service* InvokedMethod::parent() const noexcept
{
    return static_cast<const Service*>(Entity::parent());
}

Service* InvokedMethod::parent() noexcept
{
    return static_cast<Service*>(Entity::parent());
}
} // namespace busrpc
