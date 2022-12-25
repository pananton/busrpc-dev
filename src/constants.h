#pragma once

#include <cstdint>

/// \namespace doc_cmd Names of the supported documentation commands.
/// \file constants.h Constants used by development tool classes and functions.

namespace busrpc {

/// Name of the protobuf file, which should contain built-in busrpc types.
inline constexpr const char* Busrpc_Builtin_File = "busrpc.proto";

/// Name of the protobuf file, which should contain namespace descriptor.
inline constexpr const char* Namespace_Desc_File = "namespace.proto";

/// Name of the protobuf file, which should contain class descriptor.
inline constexpr const char* Class_Desc_File = "class.proto";

/// Name of the protobuf file, which should contain method descriptor.
inline constexpr const char* Method_Desc_File = "method.proto";

/// Name of the protobuf file, which should contain service descriptor.
inline constexpr const char* Service_Desc_File = "service.proto";

/// Predefined \ref Project entity name.
inline constexpr const char* Project_Entity_Name = "busrpc";

/// Predefined \ref Api entity name.
inline constexpr const char* Api_Entity_Name = "api";

/// Predefined \ref Services entity name.
inline constexpr const char* Services_Entity_Name = "services";

/// Predefined field name for the error code in the exception type.
inline constexpr const char* Exception_Code_Field_Name = "code";

/// Predefined field name for the object identifier in the call message type.
inline constexpr const char* Call_Message_Object_Id_Field_Name = "object_id";

/// Predefined field name for the serialized method parameters in the call message type.
inline constexpr const char* Call_Message_Params_Field_Name = "params";

/// Predefined field name for the serialized return value in the result message type.
inline constexpr const char* Result_Message_Retval_Field_Name = "retval";

/// Predefined field name for the exception in the result message type.
inline constexpr const char* Result_Message_Exception_Field_Name = "exception";

/// Protobuf custom message option for marking structure as hashed for the purpose of the busrpc endpoint encoding.
inline constexpr const char* Message_Option_Hashed = "hashed_struct";

/// Protobuf custom field option for marking field as observable.
inline constexpr const char* Field_Option_Observable = "observable";

/// Protobuf custom field option for marking field as hashed for the purpose of the busrpc endpoint encoding.
inline constexpr const char* Field_Option_Hashed = "hashed";

/// Protobuf custom field option for setting default field value.
inline constexpr const char* Field_Option_Default_Value = "default_value";

/// Predefined \ref Project description.
inline constexpr const char* Project_Entity_Description = "Busrpc project.";

/// Predefined \ref Api description.
inline constexpr const char* Api_Entity_Description = "Busrpc project API.";

/// Predefined \ref Services description.
inline constexpr const char* Services_Entity_Description = "Busrpc project services.";

/// Predefined name for the busrpc API error code enumeration.
inline constexpr const char* Errc_Enum_Name = "Errc";

/// Minimum allowed field number.
/// \note See [here](https://developers.google.com/protocol-buffers/docs/proto3#assigning_field_numbers)).
inline constexpr int32_t Min_Field_Number = 1;

/// Maximum allowed field number.
/// \note See [here](https://developers.google.com/protocol-buffers/docs/proto3#assigning_field_numbers)).
inline constexpr int32_t Max_Field_Number = 536870911;

/// Start of the reserved field number range.
/// \note See [here](https://developers.google.com/protocol-buffers/docs/proto3#assigning_field_numbers)).
inline constexpr int32_t Reserved_Field_Number_Range_Start = 19000;

/// End of the reserved field number range.
/// \note See [here](https://developers.google.com/protocol-buffers/docs/proto3#assigning_field_numbers)).
inline constexpr int32_t Reserved_Field_Number_Range_End = 19999;

/// Default description for busrpc \c ObjectId predefined structure.
inline constexpr const char* Default_ObjectId_Description = "Object identifier.";

/// Default description for busrpc \c Params predefined structure.
inline constexpr const char* Default_Params_Description = "Method parameters.";

/// Default description for busrpc \c Retval predefined structure.
inline constexpr const char* Default_Retval_Description = "Method return value.";

/// Default description for busrpc \c Static predefined structure.
inline constexpr const char* Default_Static_Description = "Static method marker.";

/// Default description for busrcp \c Config predefined structure.
inline constexpr const char* Default_Config_Description = "Service configuration.";

/// Default description for busrpc \c Implements predefined structure.
inline constexpr const char* Default_Implements_Description = "Methods implemented by the service.";

/// Default description for busrpc \c Invokes predefined structure.
inline constexpr const char* Default_Invokes_Description = "Methods invoked by the service.";

namespace doc_cmd {
/// Service author.
inline constexpr const char* Service_Author = "author";

/// Service contact email.
inline constexpr const char* Service_Email = "email";

/// Service sources/documentation URL.
inline constexpr const char* Service_Url = "url";

/// Value of the object identifier or some observable parameter accepted by the implementor.
inline constexpr const char* Accepted_Value = "accept";
} // namespace doc_cmd
} // namespace busrpc
