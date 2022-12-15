#pragma once

#include "entities/class.h"
#include "entities/entity.h"
#include "entities/struct.h"

#include <map>
#include <string>

/// \file method.h Method entity.

namespace busrpc {

class Class;

/// Method entity.
class Method: public GeneralCompositeEntity {
public:
    using GeneralCompositeEntity::addStruct;
    using GeneralCompositeEntity::addEnum;

    /// Class where method is defined.
    const Class* parent() const noexcept;

    /// Class where method is defined.
    Class* parent() noexcept;

    /// Method descriptor.
    const Struct* descriptor() const noexcept { return descriptor_; }

    /// Method parameters.
    /// \note \c nullptr if method does not have parameters.
    const Struct* params() const noexcept { return params_; }

    /// Method return value.
    /// \note \c nullptr if method is one-way.
    const Struct* retval() const noexcept { return retval_; }

    /// Flag indicating whether method is static.
    bool isStatic() const noexcept { return isStatic_; }

    /// Flag indicating whether methods has parameters.
    bool hasParams() const noexcept { return params() != nullptr; }

    /// Flag indicating whether method is one-way (i.e., does not have a retval).
    bool isOneway() const noexcept { return retval() == nullptr; }

protected:
    /// Create method entity.
    Method(CompositeEntity* cls, const std::string& name);

private:
    friend class CompositeEntity;
    void onNestedEntityAdded(Entity* entity);

    const Struct* descriptor_ = nullptr;
    const Struct* params_ = nullptr;
    const Struct* retval_ = nullptr;
    bool isStatic_ = false;
};
} // namespace busrpc
