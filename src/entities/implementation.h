#pragma once

#include "entities/entity.h"
#include "entities/project.h"
#include "entities/service.h"

#include <string>

/// \file implementation.h Project services entity.

namespace busrpc {

class Project;
class Service;

/// Entity representing project API implementation details.
class Implementation: public GeneralCompositeEntity {
public:
    using GeneralCompositeEntity::addStruct;
    using GeneralCompositeEntity::addEnum;

    /// Project to which entity belongs.
    const Project* parent() const noexcept;

    /// Project to which entity belongs.
    Project* parent() noexcept;

    /// Project services.
    const EntityContainer<Service>& services() const noexcept { return implementation_; }

    /// Add service.
    /// \throws name_conflict_error if nested entity with the same name already exists
    Service* addService(const std::string& name);

protected:
    /// Create services entity.
    explicit Implementation(CompositeEntity* project);

private:
    friend class CompositeEntity;

    EntityContainer<Service> implementation_;
};
} // namespace busrpc
