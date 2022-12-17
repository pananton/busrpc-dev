#pragma once

#include "entities/api.h"
#include "entities/class.h"
#include "entities/entity.h"
#include "entities/struct.h"

#include <string>

/// \file namespace.h Namespace entity.

namespace busrpc {

class Api;
class Class;

/// Namespace entity.
class Namespace: public GeneralCompositeEntity {
public:
    using GeneralCompositeEntity::addStruct;
    using GeneralCompositeEntity::addEnum;

    /// API where namespace is defined.
    const Api* parent() const noexcept;

    /// API where namespace is defined.
    Api* parent() noexcept;

    /// Namespace descriptor.
    const Struct* descriptor() const noexcept { return descriptor_; }

    /// Namespace classes.
    const EntityContainer<Class>& classes() const noexcept { return classes_; }

    /// Add class.
    /// \throws name_conflict_error if nested entity with the same name already exists
    Class* addClass(const std::string& name);

protected:
    /// Create namespace entity.
    Namespace(CompositeEntity* api, const std::string& name);

private:
    friend class CompositeEntity;
    void onNestedEntityAdded(Entity* entity);

    const Struct* descriptor_ = nullptr;
    EntityContainer<Class> classes_;
};
} // namespace busrpc
