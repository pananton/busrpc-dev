#pragma once

#include "entities/entity.h"

#include <map>
#include <string>

/// \file api.h Entity representing complete busrpc API.

namespace busrpc {

class Namespace;
class Struct;
class Enum;
class Project;
class Parser;

class API: public Entity {
public:
    /// API namespaces ordered by name.
    const std::map<std::string, const Namespace*>& namespaces() const noexcept { return namespaces_; }

    /// Global structs ordered by name.
    const std::map<std::string, const Struct*>& structs() const noexcept { return globalStructs_; }

    /// Global enumerations ordered by name.
    const std::map<std::string, const Enum*>& enums() const noexcept { return globalEnums_; }

    /// Project which owns the API.
    const Project* parent() const noexcept;

private:
    friend class Parser;

    API(): Entity(EntityType::Api) { }

    std::map<std::string, const Namespace*> namespaces_;
    std::map<std::string, const Struct*> globalStructs_;
    std::map<std::string, const Enum*> globalEnums_;
};
} // namespace busrpc