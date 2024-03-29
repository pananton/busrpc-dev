#pragma once

#include "entities/entity.h"
#include "entities/method.h"
#include "entities/namespace.h"
#include "entities/struct.h"

#include <string>

/// \file class.h Class entity.

namespace busrpc {

class Namespace;
class Method;

/// Class entity.
class Class: public GeneralCompositeEntity {
public:
    using GeneralCompositeEntity::addStruct;
    using GeneralCompositeEntity::addEnum;

    /// Namespace where class is defined.
    const Namespace* parent() const noexcept;

    /// Namespace where class is defined.
    Namespace* parent() noexcept;

    /// Class descriptor.
    const Struct* descriptor() const noexcept { return descriptor_; }

    /// Object identifier.
    /// \note \c nullptr if class is static, i.e. does not have objects.
    const Struct* objectId() const noexcept { return objectId_; }

    /// Flag indicating whether class is static.
    bool isStatic() const noexcept { return objectId() == nullptr; }

    /// Class methods.
    const EntityContainer<Method>& methods() const noexcept { return methods_; }

    /// Add method.
    /// \throws name_conflict_error if nested entity with the same name already exists
    Method* addMethod(const std::string& name);

protected:
    /// Create class entity.
    Class(CompositeEntity* ns, const std::string& name);

private:
    friend class CompositeEntity;
    void onNestedEntityAdded(Entity* entity);

    const Struct* descriptor_ = nullptr;
    const Struct* objectId_ = nullptr;
    EntityContainer<Method> methods_;
};
} // namespace busrpc
