#pragma once

#include "entities/entity.h"

#include <map>
#include <string>

/// \file service.h Busrpc service.

namespace busrpc {

class Struct;
class Enum;
class Services;
class Parser;

/// Busrpc service.
class Service: public Entity {
public:
    /// Service config.
    /// \note Can be \c nullptr, if config is not defined for a service.
    const Struct* config() const noexcept { return config_; }

    /// Class methods ordered by name.
    const std::map<std::string, const Method*>& methods() const noexcept { return methods_; }

    /// Structures defined in the class directory (ordered by name).
    const std::map<std::string, const Struct*>& structs() const noexcept { return nestedStructs_; }

    /// Enumerations defined in the class directory (ordered by name).
    const std::map<std::string, const Enum*>& enums() const noexcept { return nestedEnums_; }

    /// Flag indicating whether class is static.
    bool isStatic() const noexcept { return objectId_ == nullptr; }

    /// Namespace where class is defined.
    const Namespace* parent() const noexcept;

private:
    friend class Parser;

    Class(): Entity(EntityType::Class) { }

    const Struct* objectId_ = nullptr;
    std::map<std::string, const Method*> methods_ = {};
    std::map<std::string, const Struct*> nestedStructs_ = {};
    std::map<std::string, const Enum*> nestedEnums_ = {};
};
} // namespace busrpc