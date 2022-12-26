#pragma once

#include "entities/entity.h"
#include "entities/enum.h"
#include "entities/namespace.h"
#include "entities/project.h"
#include "entities/struct.h"

#include <string>

/// \file api.h Project API entity.

namespace busrpc {

class Project;
class Namespace;

/// API entity.
class Api: public GeneralCompositeEntity {
public:
    using GeneralCompositeEntity::addStruct;
    using GeneralCompositeEntity::addEnum;

    /// Project to which API belongs.
    const Project* parent() const noexcept;

    /// Project to which API belongs.
    Project* parent() noexcept;

    /// API namespaces.
    const EntityContainer<Namespace>& namespaces() const noexcept { return namespaces_; }

    /// Add namespace.
    /// \throws name_conflict_error if nested entity with the same name already exists
    Namespace* addNamespace(const std::string& name);

protected:
    /// Create API entity.
    explicit Api(CompositeEntity* project);

private:
    friend class CompositeEntity;

    EntityContainer<Namespace> namespaces_;
};
} // namespace busrpc
