#pragma once

#include "entities/entity.h"

#include <map>
#include <string>

/// \file enum.h Enumeration.

namespace busrpc {

class Constant;
class Parser;

/// Enumeration.
/// \note Represents protobuf \c enum type.
class Enum: public Entity {
public:
    /// Protobuf package name where enumeration is defined.
    const std::string& package() const noexcept { return package_; }

    /// Enumeration constants ordered by their names.
    const std::map<std::string, const Constant*>& constants() const noexcept { return constants_; }

private:
    friend class Parser;

    Enum(): Entity(EntityType::Enum) { }

    std::string package_ = {};
    std::map<std::string, const Constant*> constants_;
};
} // namespace busrpc
