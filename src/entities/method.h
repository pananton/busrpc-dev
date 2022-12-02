#pragma once

#include "entities/entity.h"

#include <map>
#include <string>
#include <type_traits>

/// \file method.h Busrpc method.

namespace busrpc {

class Struct;
class Enum;
class Class;
class Parser;

/// Busrpc method.
class Method: public Entity {
public:
    /// Method descriptor.
    const Struct* descriptor() const noexcept;

    /// Method parameters.
    /// \note \c nullptr if method does not have parameters.
    const Struct* params() const noexcept;

    /// Method return value.
    /// \note \c nullptr if method is one-way.
    const Struct* retval() const noexcept;

    /// Structures defined in the method directory (ordered by name).
    const std::map<std::string, const Struct*>& structs() const noexcept { return nestedStructs_; }

    /// Enumerations defined in the method directory (ordered by name).
    const std::map<std::string, const Enum*>& enums() const noexcept { return nestedEnums_; }

    /// Flag indicating whether method is static.
    bool isStatic() const noexcept;

    /// Class where method is defined.
    const Class* parent() const noexcept;

private:
    friend class Parser;

    Method(): Entity(EntityType::Method) { }

    std::map<std::string, const Struct*> nestedStructs_ = {};
    std::map<std::string, const Enum*> nestedEnums_ = {};
};
} // namespace busrpc