#pragma once

#include "entities/entity.h"
#include "entities/method.h"
#include "entities/namespace.h"
#include "entities/struct.h"

#include <map>
#include <string>
#include <type_traits>

/// \file class.h Class entity.

namespace busrpc {

class Namespace;
class Method;

/// Class entity.
class Class: public GeneralCompositeEntity {
public:
    using GeneralCompositeEntity::addStruct;
    using GeneralCompositeEntity::addEnum;

    /// Class descriptor.
    const Struct* descriptor() const noexcept { return descriptor_; }

    /// Object identifier.
    /// \note \c nullptr if class is static, i.e. does not have objects.
    const Struct* objectId() const noexcept { return objectId_; }

    /// Flag indicating whether class is static.
    bool isStatic() const noexcept { return objectId() == nullptr; }

    /// Class methods.
    const std::map<std::string, const Method*>& methods() const noexcept { return methods_; }

    /// Namespace where class is defined.
    const Namespace* parent() const noexcept;

    /// Namespace where class is defined.
    Namespace* parent() noexcept;

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
    std::map<std::string, const Method*> methods_;
};
} // namespace busrpc
