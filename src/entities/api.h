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

    /// Network message representing API call.
    const Struct* callMessage() const noexcept { return callMessage_; }

    /// Network message representing API call result.
    const Struct* resultMessage() const noexcept { return resultMessage_; }

    /// API common exception type.
    const Struct* exception() const noexcept { return exception_; }

    /// API error code enumeration.
    /// \note Provides extended information about API exception.
    const Enum* errc() const noexcept { return errc_; }

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
    void onNestedEntityAdded(Entity* entity);

    const Struct* callMessage_ = nullptr;
    const Struct* resultMessage_ = nullptr;
    const Struct* exception_ = nullptr;
    const Enum* errc_ = nullptr;
    EntityContainer<Namespace> namespaces_;
};
} // namespace busrpc
