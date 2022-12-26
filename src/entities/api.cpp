#include "entities/api.h"

#include <cassert>

namespace busrpc {

Api::Api(CompositeEntity* project):
    GeneralCompositeEntity(project, EntityTypeId::Api, Api_Entity_Name, {{Api_Entity_Description}, {}})
{
    assert(dynamic_cast<Project*>(this->parent()));
}

const Project* Api::parent() const noexcept
{
    return static_cast<const Project*>(GeneralCompositeEntity::parent());
}

Project* Api::parent() noexcept
{
    return static_cast<Project*>(GeneralCompositeEntity::parent());
}

Namespace* Api::addNamespace(const std::string& name)
{
    Namespace* ns = addNestedEntity<Namespace>(name);
    namespaces_.insert(ns);
    return ns;
}
} // namespace busrpc
