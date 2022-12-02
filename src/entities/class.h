#pragma once

#include "entities/entity.h"

#include <map>
#include <string>

/// \file class.h Busrpc class.

namespace busrpc {

class Method;
class Struct;
class Enum;
class Namespace;
class Parser;

/// Busrpc class.
class Class: public Entity {
public:
    /// Method descriptor.
    const Struct* descriptor() const noexcept;

    /// Class object identifier.
    /// \note \c nullptr if class is static.
    const Struct* objectId() const noexcept;

    /// Class methods ordered by name.
    const std::map<std::string, const Method*>& methods() const noexcept { return methods_; }

    /// Structures defined in the class directory (ordered by name).
    const std::map<std::string, const Struct*>& structs() const noexcept { return nestedStructs_; }

    /// Enumerations defined in the class directory (ordered by name).
    const std::map<std::string, const Enum*>& enums() const noexcept { return nestedEnums_; }

    /// Flag indicating whether class is static.
    bool isStatic() const noexcept { return objectId() == nullptr; }

    /// Namespace where class is defined.
    const Namespace* parent() const noexcept;

private:
    friend class Parser;

    Class(): Entity(EntityType::Class) { }

    std::map<std::string, const Method*> methods_ = {};
    std::map<std::string, const Struct*> nestedStructs_ = {};
    std::map<std::string, const Enum*> nestedEnums_ = {};
};
} // namespace busrpc