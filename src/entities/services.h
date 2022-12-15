#pragma once

#include "entities/entity.h"
#include "entities/project.h"
#include "entities/service.h"

#include <map>
#include <string>

/// \file services.h Project services entity.

namespace busrpc {

class Project;
class Service;

/// Entity representing all project services.
class Services: public GeneralCompositeEntity {
public:
    using GeneralCompositeEntity::addStruct;
    using GeneralCompositeEntity::addEnum;

    /// Project to which entity belongs.
    const Project* parent() const noexcept;

    /// Project to which entity belongs.
    Project* parent() noexcept;

    /// Project services.
    const std::map<std::string, const Service*>& services() const noexcept { return services_; }

    /// Add service.
    /// \throws name_conflict_error if nested entity with the same name already exists
    Service* addService(const std::string& name);

protected:
    /// Create services entity.
    Services(CompositeEntity* project);

private:
    friend class CompositeEntity;

    std::map<std::string, const Service*> services_;
};
} // namespace busrpc
