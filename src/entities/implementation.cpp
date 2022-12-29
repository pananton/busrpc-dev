#include "entities/service.h"

#include <cassert>

namespace busrpc {

Implementation::Implementation(CompositeEntity* project):
    GeneralCompositeEntity(project,
                           EntityTypeId::Implementation,
                           Implementation_Entity_Name,
                           {{Implementation_Entity_Description}, {}})
{
    assert(dynamic_cast<Project*>(this->parent()));
}

const Project* Implementation::parent() const noexcept
{
    return static_cast<const Project*>(GeneralCompositeEntity::parent());
}

Project* Implementation::parent() noexcept
{
    return static_cast<Project*>(GeneralCompositeEntity::parent());
}

Service* Implementation::addService(const std::string& name)
{
    Service* service = addNestedEntity<Service>(name);
    implementation_.insert(service);
    return service;
}
} // namespace busrpc
