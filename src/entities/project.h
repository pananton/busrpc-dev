#pragma once

#include "entities/api.h"
#include "entities/entity.h"
#include "entities/services.h"

#include <filesystem>
#include <string>
#include <unordered_map>

/// \file project.h Project entity.

namespace busrpc {

class Api;
class Services;

/// Project entity.
class Project: public CompositeEntity {
public:
    /// Create project entity.
    explicit Project(std::filesystem::path root = std::filesystem::current_path());

    /// Project root directory.
    const std::filesystem::path& root() const noexcept { return root_; }

    /// Project API.
    const Api* api() const noexcept { return api_; }

    /// Project services.
    /// \throws name_conflict_error if api entity is already added.
    const Services* services() const noexcept { return services_; }

    /// Find entity in the project by the distinguished name \a dname.
    /// \note Distinguished names of all entities start with a common prefix "busrpc". This prefix may be omitted
    ///       from \a dname.
    const Entity* find(const std::string& dname) const noexcept;

    /// Add project API.
    /// \throws name_conflict_error if API entity is already added.
    Api* addApi();

    /// Add project services.
    /// \throws name_conflict_error if services entity is already added.
    Services* addServices();

private:
    void onNestedEntityAdded(Entity* entity);

    std::filesystem::path root_;
    const Api* api_ = nullptr;
    const Services* services_ = nullptr;

    std::unordered_map<std::string, const Entity*> entityDirectory_;
};
} // namespace busrpc
