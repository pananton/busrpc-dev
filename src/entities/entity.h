#pragma once

#include "exception.h"
#include "types.h"

#include <concepts>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <type_traits>
#include <vector>

/// \dir entities Busprc API entities
/// \file entity.h Base classes for busrpc entities.

namespace busrpc {

class CompositeEntity;
class Enum;
class Struct;

/// Entity base class.
class Entity {
public:
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    Entity(Entity&&) = default;
    Entity& operator=(Entity&&) = default;

    /// Entity type.
    EntityTypeId type() const noexcept { return type_; }

    /// Entity name (non-unique).
    const std::string& name() const noexcept { return name_; }

    /// Directory where entity is defined.
    const std::filesystem::path& dir() const noexcept { return dir_; }

    /// Entity description.
    /// \note Result \c std::vector contains lines of block comment bound to the entity in a protobuf file except for
    ///       lines which start with a busrpc documentation command.
    const std::vector<std::string>& description() const noexcept { return description_; }

    /// Entity brief description (the first line of the \ref description of exists).
    /// \note Empty if description is not specified and \ref description return empty vector.
    const std::string& briefDescription() const noexcept { return briefDescription_; }

    /// Entity documentation commands.
    /// \note Key in the return type is documentation command name and value is command argument.
    const std::multimap<std::string, std::string>& docCommands() const noexcept { return docCommands_; }

    /// Parent entity.
    /// \note \c nullptr for \ref Project entity.
    const CompositeEntity* parent() const noexcept { return parent_; }

    /// Parent entity.
    /// \note \c nullptr for \ref Project entity.
    CompositeEntity* parent() noexcept { return parent_; }

    /// Destructor.
    virtual ~Entity() = default;

protected:
    /// Create entity.
    Entity(CompositeEntity* parent, EntityTypeId type, const std::string& name, const std::string& blockComment = {});

    /// Set entity documentation.
    void setDocumentation(const std::vector<std::string>& description,
                          const std::string briefDescription,
                          const std::multimap<std::string, std::string> docCommands)
    {
        description_ = description;
        briefDescription_ = briefDescription;
        docCommands_ = docCommands;
    }

private:
    void parseBlockComment(const std::string& blockComment);

    CompositeEntity* parent_;
    EntityTypeId type_;
    std::string name_;

    std::filesystem::path dir_;
    std::vector<std::string> description_;
    std::string briefDescription_;
    std::multimap<std::string, std::string> docCommands_;

    std::map<std::string, const Entity*> nested_;
    std::queue<std::shared_ptr<Entity>> storage_;

    std::function<void(const Entity*)> onNestedEntityAdded_;
};

/// Entity concept.
template<typename T>
concept EntityConcept = std::is_base_of_v<Entity, T>;

/// Entity that has unique distinguished name.
class DistinguishedEntity: public Entity {
public:
    /// Entity distinguished name (uniquely identifies the entity).
    /// \note Distinguished name consists of the dot-separated names of entity's parents and \ref name of the entity
    ///       itself. Parents are included in the distinguished name as they recede from the root of the entity tree
    ///       (i.e., \ref Project entity), for example:
    ///           <tt>busrpc.api.namespace.class.method.Struct.NestedStruct.NestedEnum.NestedEnumConstant</tt>
    /// \note Distinguished names of entities that represent protobuf types (\ref Struct  representing a \c message
    /// and
    ///       \ref Enum representing an \c enum) match fully-qualified names of a corresponding protobuf types.
    const std::string& dname() const noexcept { return dname_; }

protected:
    /// Create distinguished entity.
    DistinguishedEntity(CompositeEntity* parent,
                        EntityTypeId type,
                        const std::string& name,
                        const std::string& blockComment = {});

private:
    std::string dname_;
};

/// Distinguished entity concept.
template<typename T>
concept DistinguishedEntityConcept = std::is_base_of_v<DistinguishedEntity, T>;

class CompositeEntity;

/// Composite entity concept.
template<typename T>
concept CompositeEntityConcept = std::is_base_of_v<CompositeEntity, T>;

/// Simple entity concept.
template<typename T>
concept SimpleEntityConcept = std::is_base_of_v<Entity, T> && !std::is_base_of_v<CompositeEntity, T>;

/// Entity that has nested entities.
class CompositeEntity: public DistinguishedEntity {
public:
    /// Immideately nested entites ordered by name.
    const std::map<std::string, const Entity*>& nested() const noexcept { return nested_; }

protected:
    /// Callback invoked whenever nested entity is added.
    /// \note This callback is invoked for all entities which derive from the current one, not only entities that
    ///       are immediately nested to it.
    using NestedEntityAddedCallback = std::function<void(Entity*)>;

    /// Create composite entity.
    CompositeEntity(CompositeEntity* parent,
                    EntityTypeId type,
                    const std::string& name,
                    const std::string& blockComment = {}):
        DistinguishedEntity(parent, type, name, blockComment),
        storage_{},
        nested_{},
        onNestedEntityAdded_(parent ? parent->onNestedEntityAdded_ : NestedEntityAddedCallback{})
    { }

    /// Create entity and add it to the list of nested entites.
    /// \tparam TArgs arguments that forwarded to \c TEntity constructor.
    /// \throws name_conflict_error if entity with the same name is already added
    template<EntityConcept TEntity, typename... TArgs>
    TEntity* addNestedEntity(TArgs&&... args)
    {
        std::shared_ptr<TEntity> entityPtr(new TEntity(this, std::forward<TArgs>(args)...));
        auto alreadyExists = !nested_.emplace(entityPtr->name(), entityPtr.get()).second;

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
    void setNestedEntityAddedCallback(NestedEntityAddedCallback callback)
    {
        if (onNestedEntityAdded_) {
            onNestedEntityAdded_ = [this,
                                    newCallback = std::move(callback),
                                    originalCallback = std::move(onNestedEntityAdded_)](Entity* addedEntity) {
                newCallback(addedEntity);
                originalCallback(addedEntity);
            };
        } else {
            onNestedEntityAdded_ = callback;
        }
    }

private:
    template<EntityConcept TEntity, typename... TArgs>
    TEntity* addNestedEntityImpl(TArgs&&... args)
    {
        std::shared_ptr<TEntity> entityPtr(new TEntity(this, std::forward<TArgs>(args)...));
        auto alreadyExists = !nested_.emplace(entityPtr->name(), entityPtr.get()).second;

        if (!alreadyExists) {
            storage_.push(entityPtr);
        } else {
            throw name_conflict_error(type(), dname(), entityPtr->name());
        }

        return entityPtr.get();
    }

    std::map<std::string, const Entity*> nested_;
    std::queue<std::shared_ptr<Entity>> storage_;
    NestedEntityAddedCallback onNestedEntityAdded_;
};

/// Composite entity that supports structures and enumerations as nested types.
class GeneralCompositeEntity: public CompositeEntity {
public:
    /// Immediately nested structures ordered by name.
    const std::map<std::string, const Struct*>& structs() const noexcept { return structs_; }

    /// Immediately nested enumerations ordered by name.
    const std::map<std::string, const Enum*>& enums() const noexcept { return enums_; }

protected:
    /// Create general composite entity.
    GeneralCompositeEntity(CompositeEntity* parent,
                           EntityTypeId type,
                           const std::string& name,
                           const std::string& blockComment = {}):
        CompositeEntity(parent, type, name, blockComment)
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
                      const std::string& blockComment = {});

    /// Add nested enumeration.
    /// \throws name_conflict_error if entity with the same name is already added
    /// \throws entity_error if \a filename is invalid or contains directory components
    /// \note Parameter \a filename contains name of the file for the corresponding protobuf \c enum type.
    /// If
    ///       general composite entity represents \ref Struct entity, then this parameter is ignored and
    ///       parent structure filename is used instead of if.
    Enum* addEnum(const std::string& name, const std::string& filename, const std::string& blockComment = {});

private:
    std::map<std::string, const Struct*> structs_ = {};
    std::map<std::string, const Enum*> enums_ = {};
};
} // namespace busrpc
