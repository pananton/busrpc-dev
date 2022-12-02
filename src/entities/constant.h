#pragma once

#include "entities/entity.h"

#include <cstdint>

/// \file constant.h Enumeration constant.

namespace busrpc {

class Enum;
class Parser;

/// Enumeration constant.
class Constant: public Entity {
public:
    /// Value of the constant.
    int32_t value() const noexcept { return value_; }

    /// Enumeration to which constant belongs.
    const Enum* parent() const noexcept;

private:
    friend class Parser;

    Constant(): Entity(EntityType::Constant) { }

    int32_t value_ = 0;
};
} // namespace busrpc
