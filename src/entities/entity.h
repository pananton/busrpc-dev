#pragma once

#include "exception.h"
#include "types.h"

#include <concepts>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

/// \dir entities Busprc API entities
/// \file entity.h Base classes for busrpc entities.

namespace busrpc {

class CompositeEntity;
class Enum;
class Struct;

/// Entity documentation.
class EntityDocs {
public:
    /// Create entity documentation from the \a description and \a commands.
    /// \warning If vector representing documentation command value is empty, then constructor will add empty string
    ///          as a command value.
    /// \note Brief description is the first item of the \a description.
    /// \note Leading and trailing empty/whitespace-only lines in the \a description are skipped. Also leading and
    ///       trailing whitespaces in the command values are trimmed.
    EntityDocs(const std::vector<std::string>& description = {},
               std::map<std::string, std::vector<std::string>> commands = {});

    /// Create entity documentation from the protobuf file block comment.
    /// \note Parameter \a blockComment should not contain characters, which start a comment line (like '//', etc.).
    /// \note Leading and trailing empty/whitespace-only lines in the parsed description are skipped.
    explicit EntityDocs(const std::string& blockComment);

    /// Entity description.
    const std::vector<std::string>& description() const noexcept { return description_; }

    /// Entity brief description.
    const std::string& brief() const noexcept { return brief_; }

    /// Entity documentation commands ordered by command name.
    /// \note Each documentation command is a name-value pair. Because same documentation command may be specified
    ///       more that once for an entity, returned \c map uses \c vector to store all values.
    const std::map<std::string, std::vector<std::string>>& commands() const noexcept { return commands_; }

private:
    std::vector<std::string> description_;
    std::string brief_;
    std::map<std::string, std::vector<std::string>> commands_;
};

/// Entity base class.
class Entity {
public:
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    Entity(Entity&&) = delete;
    Entity& operator=(Entity&&) = delete;

    /// Parent entity.
    /// \note \c nullptr for \ref Project entity.
    const CompositeEntity* parent() const noexcept { return parent_; }

    /// Parent entity.
    /// \note \c nullptr for \ref Project entity.
    CompositeEntity* parent() noexcept { return parent_; }

    /// Entity type.
    EntityTypeId type() const noexcept { return type_; }

    /// Entity name (non-unique).
    const std::string& name() const noexcept { return name_; }

    /// Entity distinguished name (uniquely identifies the entity).
    /// \note Distinguished name consists of the dot-separated names of entity's parents and \ref name of the entity
    ///       itself. Parents are included in the distinguished name as they recede from the root of the entity tree
    ///       (i.e., \ref Project entity), for example:
    ///           <tt>busrpc.api.namespace.class.method.Struct.NestedStruct.NestedEnum.NestedEnumConstant</tt>
    /// \note Distinguished names of entities that represent protobuf types (\ref Struct  representing a \c message
    ///       and \ref Enum representing an \c enum) match fully-qualified names of a corresponding protobuf types.
    const std::string& dname() const noexcept { return dname_; }

    /// Directory where entity is defined, relative to busrpc project directory.
    const std::filesystem::path& dir() const noexcept { return dir_; }

    /// Entity documentation.
    const EntityDocs& docs() const noexcept { return docs_; }

    /// Destructor.
    virtual ~Entity() = default;

protected:
    /// Create entity.
    Entity(CompositeEntity* parent, EntityTypeId type, const std::string& name, EntityDocs docs);

    /// Set entity documentation.
    void setDocumentation(EntityDocs docs) noexcept { docs_ = std::move(docs); }

private:
    CompositeEntity* parent_;
    EntityTypeId type_;
    std::string name_;
    std::string dname_;
    std::filesystem::path dir_;
    EntityDocs docs_;

    std::map<std::string, const Entity*> nested_;
    std::queue<std::shared_ptr<Entity>> storage_;

    std::function<void(const Entity*)> onNestedEntityAdded_;
};

/// Entity concept.
template<typename T>
concept EntityConcept = std::is_base_of_v<Entity, T>;

/// Composite entity concept.
template<typename T>
concept CompositeEntityConcept = std::is_base_of_v<CompositeEntity, T>;

/// Simple entity concept.
template<typename T>
concept SimpleEntityConcept = std::is_base_of_v<Entity, T> && !std::is_base_of_v<CompositeEntity, T>;

/// Descendant order of entity names.
struct OrderEntitiesByNameAsc {
    using ComparedType = const Entity*;
    using is_transparent = void;

    /// Return \c true if name of the entity pointed by \a lhs is less than name of the entity pointed by \a rhs.
    bool operator()(const ComparedType& lhs, const ComparedType& rhs) const noexcept
    {
        return lhs->name() < rhs->name();
    }

    /// Return \c true if name of the entity pointed by \a lhs is less than \a rhs.
    bool operator()(const ComparedType& lhs, std::string_view rhs) const noexcept { return lhs->name() < rhs; }

    /// Return \c true if \a lhs is less than name of the entity pointed by \a rhs.
    bool operator()(std::string_view lhs, const ComparedType& rhs) const noexcept { return lhs < rhs->name(); }
};

/// Container for storing pointers to entities in the ascending order of entity names.
template<typename TEntity>
using EntityContainer = std::set<const TEntity*, OrderEntitiesByNameAsc>;

/// Entity that has nested entities.
class CompositeEntity: public Entity {
public:
    /// Immideately nested entites.
    const EntityContainer<Entity>& nested() const noexcept { return nested_; }

protected:
    /// Callback invoked whenever nested entity is added.
    /// \note This callback is invoked for all entities which derive from the current one, not only entities that
    ///       are immediately nested to it.
    using NestedEntityAddedCallback = std::function<void(Entity*)>;

    /// Create composite entity.
    CompositeEntity(CompositeEntity* parent, EntityTypeId type, const std::string& name, EntityDocs docs = {}):
        Entity(parent, type, name, std::move(docs)),
        storage_{},
        nested_{},
        onNestedEntityAdded_(parent ? parent->onNestedEntityAdded_ : NestedEntityAddedCallback{})
    { }

    /// Create entity and add it to the list of nested entites.
    /// \tparam TArgs arguments that forwarded to \c TEntity constructor.
    /// \throws entity_error if \a name is invalid
    /// \throws name_conflict_error if entity with the same name is already added
    template<EntityConcept TEntity, typename... TArgs>
    TEntity* addNestedEntity(TArgs&&... args)
    {
        std::shared_ptr<TEntity> entityPtr(new TEntity(this, std::forward<TArgs>(args)...));
        auto alreadyExists = !nested_.insert(entityPtr.get()).second;

        if (!alreadyExists) {
            storage_.push(entityPtr);
        } else {
            throw name_conflict_error(type(), dname(), entityPtr->name());
        }

        if (onNestedEntityAdded_) {
            onNestedEntityAdded_(entityPtr.get());
        }

        return entityPtr.get();
    }

    /// Set callback to be invoked when nested entity is added.
    /// \note This callback is invoked for all entities which derive from the current one, not only entities that
    ///       are immediately nested to it.
    /// \note If callback is already set (for example, copied from the parent entity during object initialization),
    ///       then new function is created and is set as a callback. This function simply calls \a callback first
    ///       and then calls original callback. This mechanism allows to chain callbacks.
    void setNestedEntityAddedCallback(NestedEntityAddedCallback callback);

private:
    std::queue<std::shared_ptr<Entity>> storage_;
    EntityContainer<Entity> nested_;
    NestedEntityAddedCallback onNestedEntityAdded_;
};

/// Composite entity that supports structures and enumerations as nested types.
class GeneralCompositeEntity: public CompositeEntity {
public:
    /// Immediately nested structures ordered by name.
    const EntityContainer<Struct>& structs() const noexcept { return structs_; }

    /// Immediately nested enumerations ordered by name.
    const EntityContainer<Enum>& enums() const noexcept { return enums_; }

protected:
    /// Create general composite entity.
    GeneralCompositeEntity(CompositeEntity* parent, EntityTypeId type, const std::string& name, EntityDocs docs = {}):
        CompositeEntity(parent, type, name, std::move(docs))
    { }

    /// Add nested structure.
    /// \throws name_conflict_error if entity with the same name is already added
    /// \throws entity_error if \a filename is invalid or contains directory components
    /// \note Parameter \a filename contains name of the file for the corresponding protobuf \c message
    /// type. If
    ///       general composite entity represents \ref Struct entity, then this parameter is ignored and
    ///       parent structure filename is used instead of if.
    Struct* addStruct(const std::string& name,
                      const std::string& filename,
                      StructFlags flags = StructFlags::None,
                      EntityDocs docs = {});

    /// Add nested enumeration.
    /// \throws name_conflict_error if entity with the same name is already added
    /// \throws entity_error if \a filename is invalid or contains directory components
    /// \note Parameter \a filename contains name of the file for the corresponding protobuf \c enum type.
    /// If
    ///       general composite entity represents \ref Struct entity, then this parameter is ignored and
    ///       parent structure filename is used instead of if.
    Enum* addEnum(const std::string& name, const std::string& filename, EntityDocs docs = {});

private:
    EntityContainer<Struct> structs_;
    EntityContainer<Enum> enums_;
};
} // namespace busrpc
