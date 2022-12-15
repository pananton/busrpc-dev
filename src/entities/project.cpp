#include "entities/project.h"

#include <cassert>

namespace busrpc {

Project::Project(std::filesystem::path root):
    CompositeEntity(nullptr, EntityTypeId::Project, Project_Entity_Name, {{Project_Entity_Comment}, {}}),
    root_(std::move(root))
{
    setNestedEntityAddedCallback([this](Entity* entity) { onNestedEntityAdded(entity); });
    entityDirectory_[name()] = this;
}

Api* Project::addApi()
{
    Api* api = addNestedEntity<Api>();
    api_ = api;
    return api;
}

Services* Project::addServices()
{
    Services* services = addNestedEntity<Services>();
    services_ = services;
    return services;
}

const Entity* Project::find(const std::string& dname) const noexcept
{
    std::string prefix = Project_Entity_Name;
    prefix.append(1, '.');
    std::string normalized;

    if (!dname.starts_with(prefix)) {
        normalized = prefix;

        if (!dname.empty() && dname != Project_Entity_Name) {
            normalized.append(dname);
        } else {
            normalized.erase(normalized.size() - 1);
        }
    } else {
        normalized = dname;
    }

    auto it = entityDirectory_.find(normalized);
    return it != entityDirectory_.end() ? it->second : nullptr;
}

void Project::onNestedEntityAdded(Entity* entity)
{
    auto distinguishedEntity = dynamic_cast<DistinguishedEntity*>(entity);

    if (distinguishedEntity) {
        auto isAdded = entityDirectory_.emplace(distinguishedEntity->dname(), distinguishedEntity).second;
        assert(isAdded);
    }
}
} // namespace busrpc
