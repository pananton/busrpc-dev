#pragma once

#include "entities/constant.h"
#include "entities/entity.h"

#include <filesystem>
#include <string>

/// \file enum.h Enumeration entity.

namespace busrpc {

class Constant;

/// Enumeration entity.
/// \note Represents protobuf \c enum type.
class Enum: public CompositeEntity {
public:
    /// Protobuf package for the corresponding \c enum protobuf type.
    const std::string& package() const noexcept { return package_; }

    /// File for the corresponding \c enum protobuf type.
    /// \note Returned value is comprised of \ref Entity::dir value and a filename specified when enumeration
    ///       was created.
    const std::filesystem::path& file() const noexcept { return file_; }

    /// Enumeration constants ordered by their names.
    const EntityContainer<Constant>& constants() const noexcept { return constants_; }

    /// Add enumeration constant.
    /// \throws name_conflict_error if constant with the same name is already added
    Constant* addConstant(const std::string& name, int32_t value, EntityDocs docs = {});

protected:
    /// Create enumeration entity.
    Enum(CompositeEntity* parent, const std::string& name, const std::string& filename, EntityDocs docs);

private:
    friend class CompositeEntity;

    std::string package_;
    std::filesystem::path file_;
    EntityContainer<Constant> constants_;
};
} // namespace busrpc
