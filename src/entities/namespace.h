#pragma once

#include "entities/entity.h"

#include <map>
#include <string>

/// \file namespace.h Busrpc namespace.

namespace busrpc {

class Class;
class Struct;
class Enum;
class API;
class Parser;

/// Busrpc namespace.
class Namespace: public Entity {
public:
    /// Namespace classes ordered by name.
    const std::map<std::string, const Class*>& classes() const noexcept { return classes_; }

    /// Structures defined in the namespace directory (ordered by name).
    const std::map<std::string, const Struct*>& structs() const noexcept { return nestedStructs_; }

    /// Enumerations defined in the namespace directory (ordered by name).
    const std::map<std::string, const Enum*>& enums() const noexcept { return nestedEnums_; }

    /// API to which namespace belongs.
    const API* parent() const noexcept;

private:
    friend class Parser;

    Namespace(): Entity(EntityType::Namespace) { }

    std::map<std::string, const Class*> classes_ = {};
    std::map<std::string, const Struct*> nestedStructs_ = {};
    std::map<std::string, const Enum*> nestedEnums_ = {};
};
} // namespace busrpc