#include "entities/service.h"

#include <cassert>

namespace busrpc {

Services::Services(CompositeEntity* project):
    GeneralCompositeEntity(project, EntityTypeId::Services, Services_Entity_Name, {{Services_Entity_Comment}, {}})
{
    assert(dynamic_cast<Project*>(this->parent()));
}

const Project* Services::parent() const noexcept
{
    return static_cast<const Project*>(GeneralCompositeEntity::parent());
}

Project* Services::parent() noexcept
{
    return static_cast<Project*>(GeneralCompositeEntity::parent());
}

Service* Services::addService(const std::string& name)
{
    Service* service = addNestedEntity<Service>(name);
    services_.insert(service);
    return service;
}
} // namespace busrpc
