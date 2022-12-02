#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <type_traits>
#include <vector>

/// \dir entities Busprc API entities
/// \file entity.h Base class for all busrpc entities.

namespace busrpc {

class Parser;

/// Entity type.
enum class EntityType {
    Project = 1,   ///< Top-level busrpc entity.
    Api = 2,       ///< API entity, which contains all busrpc classes.
    Namespace = 3, ///< Busrpc namespace.
    Class = 4,     ///< Busrpc class.
    Method = 5,    ///< Busrpc method.
    Struct = 6,    ///< Structure (corresponds to protobuf \c message type).
    Field = 7,     ///< Structure field.
    Enum = 8,      ///< Enumeration (corresponds to protobuf \c enum type).
    Constant = 9,  ///< Enumeration constant.
    Services = 10, ///< Services entity, which contains all busrpc services.
    Service = 11   ///< Busrpc service.
};

/// Return string representation of an entity type.
/// \note \c nullptr is returned if \a type is unknown.
constexpr const char* GetEntityTypeStr(EntityType type)
{
    switch (type) {
    case EntityType::Project: return "project";
    case EntityType::Api: return "api";
    case EntityType::Namespace: return "namespace";
    case EntityType::Class: return "class";
    case EntityType::Method: return "method";
    case EntityType::Struct: return "struct";
    case EntityType::Field: return "field";
    case EntityType::Enum: return "enum";
    case EntityType::Constant: return "constant";
    case EntityType::Services: return "services";
    case EntityType::Service: return "service";
    default: return nullptr;
    }
}

/// Entity base class.
class Entity {
public:
    /// Entity type.
    EntityType type() const noexcept { return type_; }

    /// Entity name (non-unique).
    const std::string& name() const noexcept { return name_; }

    /// Entity distinguished name (uniquely identifies the entity).
    /// \note Distinguished name consists of the dot-separated names of entity's parents and \ref name of the entity
    ///       itself. Parents are included in the distinguished name as they recede from the root of the entity tree
    ///       (i.e., \ref EntityType::Project entity).
    /// \note Example of a distinguished name:
    ///       <tt>project.api.namespace.class.method.Struct.NestedStruct.NestedEnum.NestedEnumConstant</tt>
    const std::string& dname() const noexcept { return dname_; }

    /// Path to a filesystem object (file or directory) defining the entity.
    /// \note Returned path is relative to busrpc project directory.
    const std::filesystem::path& path() const noexcept { return path_; }

    /// Parent entity.
    /// \note \c nullptr for \ref Project entity.
    const Entity* parent() const noexcept { return parent_; }

    /// Entities directly nested to this entity (ordered by entity \ref name).
    const std::map<std::string, const Entity*>& nested() const noexcept { return nested_; }

    /// Entity description.
    /// \note Result \c std::vector contains lines of block comment bound to the entity in a protobuf file except for
    ///       lines which start with a busrpc documentation command (these lines are handled separately).
    /// \note Empty if description is not specified.
    const std::vector<std::string>& description() const noexcept { return description_; }

    /// Entity brief description (the first line of the \ref description of exists).
    /// \note Empty if description is not specified and \ref description return empty vector.
    std::string briefDescription() const noexcept { return !description_.empty() ? description_.front() : ""; }

    /// Destructor.
    virtual ~Entity() = default;

protected:
    Entity(EntityType type): type_(type) { }

private:
    friend class Parser;

    EntityType type_;
    std::string name_ = {};
    std::string dname_ = {};
    std::filesystem::path path_ = {};
    const Entity* parent_ = {};
    std::map<std::string, const Entity*> nested_ = {};
    std::vector<std::string> description_ = {};
};

/// Entity concept.
template<typename T>
concept EntityConcept = std::is_base_of_v<Entity, T>;
} // namespace busrpc
