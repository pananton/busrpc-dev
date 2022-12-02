#pragma once

#include "entities/entity.h"

#include <map>
#include <string>
#include <type_traits>
#include <vector>

/// \file field.h Structure.

namespace busrpc {

class Field;
class Enum;
class Parser;

/// Structure type, which determines semantic of the structure.
enum class StructType {
    General = 1,           ///< General structure.
    Call_Message = 2,      ///< Network message representing method call.
    Result_Message = 3,    ///< Network message representing method result.
    API_Desc = 4,          ///< API descriptor.
    Namespace_Desc = 5,    ///< Namespace descriptor.
    Class_Desc = 6,        ///< Class descriptor.
    Method_Desc = -7,      ///< Method descriptor.
    Object_Id = 8,         ///< Object identifier.
    Method_Params = 9,     ///< Method parameters.
    Method_Retval = 10,    ///< Method return value.
    Method_Exception = 11, ///< Method exception.
    Service_Desc = 12,     ///< Service descriptor.
    Service_Config = 13    ///< Service config.
};

/// Return string representation of a structure type.
/// \note \c nullptr is returned if \a type is unknown.
constexpr const char* GetStructTypeStr(StructType type)
{
    using enum StructType;

    switch (type) {
    case General: return "general";
    case Call_Message: return "call message";
    case Result_Message: return "result message";
    case API_Desc: return "API descriptor";
    case Namespace_Desc: return "namespace descriptor";
    case Class_Desc: return "class descriptor";
    case Method_Desc: return "method descriptor";
    case Object_Id: return "object identifier";
    case Method_Params: return "method parameters";
    case Method_Retval: return "method return value";
    case Method_Exception: return "method exception type";
    case Service_Desc: return "service descriptor";
    case Service_Config: return "service config";
    default: return nullptr;
    }
}
/// Structure.
/// \note Represents protobuf \c message type.
class Struct: public Entity {
public:
    /// Protobuf package name where enumeration is defined.
    const std::string& package() const noexcept { return package_; }

    /// Type of the structure.
    StructType structType() const noexcept { return structType_; }

    /// Structure fields ordered by name.
    const std::map<std::string, const Field*>& fields() const noexcept { return fields_; }

    /// Directly nested structures ordered by name.
    /// \note Does not return indirectly nested structures (for example, if S1 defines S2, which defines S3, then
    ///       method \ref structs called for S1 object will not return C).
    const std::map<std::string, const Struct*>& structs() const noexcept { return nestedStructs_; }

    /// Directly nested enumerations ordered by name.
    /// \note Does not return indirectly nested enumerations (for example, if S1 defines structure S2, which defines
    ///       enumeration E1, then method \ref enums called for S1 object will not return E1).
    const std::map<std::string, const Enum*>& enums() const noexcept { return nestedEnums_; }

    /// Flag indicating whether structure data is hashed when used as a busrpc endpoint component.
    bool isHashed() const noexcept { return flags_ & static_cast<std::underlying_type_t<Flags>>(Flags::Hashed); }

private:
    enum class Flags { Hashed = 1 };
    friend class Parser;

    Struct(): Entity(EntityType::Struct) { }

    StructType structType_ = static_cast<StructType>(0);
    std::map<std::string, const Field*> fields_ = {};
    std::map<std::string, const Struct*> nestedStructs_ = {};
    std::map<std::string, const Enum*> nestedEnums_ = {};
    std::underlying_type_t<Flags> flags_ = 0;
};
} // namespace busrpc
