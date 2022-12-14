#include "entities/invoked_method.h"

#include <cassert>

namespace busrpc {

InvokedMethod::InvokedMethod(CompositeEntity* service,
                             const std::string& name,
                             const std::vector<std::string>& description,
                             const std::string& briefDescription,
                             const std::multimap<std::string, std::string>& docCommands):
    Entity(service, EntityTypeId::Implemented_Method, name, "")
{
    assert(dynamic_cast<Service*>(this->parent()));
    setDocumentation(description, briefDescription, docCommands);
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
