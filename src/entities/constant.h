#pragma once

#include "entities/entity.h"
#include "entities/enum.h"

#include <cstdint>
#include <string>

/// \file constant.h Enumeration constant entity.

namespace busrpc {

class Enum;

/// Enumeration constant entity.
class Constant: public Entity {
public:
    /// Enumeration to which constant belongs.
    const Enum* parent() const noexcept;

    /// Enumeration to which constant belongs.
    Enum* parent() noexcept;

    /// Value of the constant.
    int32_t value() const noexcept { return value_; }

protected:
    /// Create enumeration constant entity.
    Constant(CompositeEntity* parent, const std::string& name, int32_t value, EntityDocs docs = {});

private:
    friend class CompositeEntity;

    int32_t value_;
};
} // namespace busrpc
