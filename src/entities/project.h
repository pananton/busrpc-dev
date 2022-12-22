#pragma once

#include "entities/api.h"
#include "entities/entity.h"
#include "entities/services.h"
#include "error_collector.h"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>

/// \file project.h Project entity.

namespace busrpc {

class Api;
class Namespace;
class Class;
class Method;
class Services;
class Service;

/// Busrpc [specification](https://github.com/pananton/busrpc-spec)-related error codes.
enum class SpecErrc {
    Missing_Api = 1,           ///< Project does not define an API.
    Missing_Builtin = 2,       ///< One of the busrpc built-in types is missing.
    Nonconforming_Builtin = 3, ///< One of the busrpc built-in types does not conform with the specification.
    Missing_Descriptor = 4,    ///< Entity (namespace, class, method or service) does not have a descriptor.
    Not_Static_Method = 5,     ///< Static class method is not marked as static.
    Not_Encodable_Type = 6,    ///< Entity must be encodable as specified in the busrpc specification.
    Not_Accessible_Type = 7,   ///< Type is not accessible in the current scope.
    Unknown_Type = 8,          ///< Type of structure/enumeration field is unknown.
    Unexpected_Type = 9,       ///< Entity type of structure/enumeration field is not as expected.
    Unknown_Method = 10,       ///< Service \c Implements or \c Invokes type references unknown method.
    Multiple_Definitions = 11, ///< Entity is defined more than once.
};

/// Busrpc [specification](https://github.com/pananton/busrpc-spec)-related warnings.
enum class SpecWarn {
    /// Entity contains unexpected nested entity (for example, some descriptor contains unknown structure).
    Unexpected_Nested_Entity = 1
};

/// Documentation-related error codes.
enum class DocErrc {
    Undocumented_Entity = 1, ///< Entity is not documented.
    Unknown_Doc_Command = 2  ///< Unknown documentation command.
};

/// Style-related error codes.
enum class StyleErrc {
    Invalid_Name_Format = 1 ///< Entity name format is invalid.
};

/// Return error category for the specification-related error codes.
const std::error_category& spec_error_category();

/// Create error code from the \ref SpecErrc value.
std::error_code make_error_code(SpecErrc errc);

/// Return error category for the specification-related warnings.
const std::error_category& spec_warn_category();

/// Create error code from the \ref SpecWarn value.
std::error_code make_error_code(SpecWarn errc);

/// Return error category for the documentation-related error codes.
const std::error_category& doc_error_category();

/// Create error code from the \ref DocErrc value.
std::error_code make_error_code(DocErrc errc);

/// Return error category for the style-related error codes.
const std::error_category& style_error_category();

/// Create error code from the \ref StyleErrc value.
std::error_code make_error_code(StyleErrc errc);

/// Project entity.
class Project: public CompositeEntity {
public:
    /// Create project entity.
    explicit Project(std::filesystem::path root = std::filesystem::current_path());

    /// Project root directory.
    const std::filesystem::path& root() const noexcept { return root_; }

    /// Project API.
    const Api* api() const noexcept { return api_; }

    /// Project services.
    /// \throws name_conflict_error if api entity is already added.
    const Services* services() const noexcept { return services_; }

    /// Find entity in the project by the distinguished name \a dname.
    /// \note Distinguished names of all entities start with a common prefix "busrpc". This prefix may be omitted
    ///       from \a dname.
    const Entity* find(const std::string& dname) const;

    /// Add project API.
    /// \throws name_conflict_error if API entity is already added.
    Api* addApi();

    /// Add project services.
    /// \throws name_conflict_error if services entity is already added.
    Services* addServices();

    /// Check project for conformance with busrpc specification.
    /// \note Uses default error collector, which assumes the following priorities of the error codes:
    ///       <tt>SpecErrc > DocsErrc > SpecWarn > StyleErrc</tt>
    ErrorCollector check() const;

    /// Check project for conformance with busrpc specification.
    void check(ErrorCollector& errorCollector) const;

private:
    void onNestedEntityAdded(Entity* entity);

    void checkApi(const Api* api, ErrorCollector& ecol) const;
    void checkErrc(const Enum* errc, ErrorCollector& ecol) const;
    void checkException(const Struct* errc, ErrorCollector& ecol) const;
    void checkCallMessage(const Struct* errc, ErrorCollector& ecol) const;
    void checkResultMessage(const Struct* errc, ErrorCollector& ecol) const;

    void checkNamespace(const Namespace* ns, ErrorCollector& ecol) const;
    void checkNamespaceDesc(const Namespace* ns, ErrorCollector& ecol) const;

    void checkClass(const Class* cls, ErrorCollector& ecol) const;
    void checkClassDesc(const Class* cls, ErrorCollector& ecol) const;
    void checkObjectId(const Class* cls, ErrorCollector& ecol) const;

    void checkMethod(const Method* method, ErrorCollector& ecol) const;
    void checkMethodDesc(const Method* method, ErrorCollector& ecol) const;

    void checkServices(const Services* services, ErrorCollector& ecol) const;
    void checkService(const Service* service, ErrorCollector& ecol) const;
    void checkServiceDesc(const Service* service, ErrorCollector& ecol) const;
    void checkServiceDeps(const Service* service, bool checkImplemented, ErrorCollector& ecol) const;

    void checkNestedStructs(const GeneralCompositeEntity* entity, ErrorCollector& ecol) const;
    void checkNestedEnums(const GeneralCompositeEntity* entity, ErrorCollector& ecol) const;

    void checkStruct(const Struct* structure, ErrorCollector& ecol) const;
    void checkField(const Field* field, ErrorCollector& ecol) const;
    void checkEnum(const Enum* enumeration, ErrorCollector& ecol) const;
    void checkConstant(const Constant* constant, ErrorCollector& ecol) const;

    void checkEntityDocumentation(const Entity* entity,
                                  ErrorCollector& ecol,
                                  const std::unordered_set<std::string>& allowedDocCommands = {}) const;
    bool isApiEntity(const Entity* entity) const noexcept;

    std::filesystem::path root_;
    const Api* api_ = nullptr;
    const Services* services_ = nullptr;

    std::unordered_map<std::string, const Entity*> entityDirectory_;
};
} // namespace busrpc

namespace std {
template<>
struct is_error_code_enum<busrpc::SpecErrc>: true_type { };

template<>
struct is_error_code_enum<busrpc::SpecWarn>: true_type { };

template<>
struct is_error_code_enum<busrpc::DocErrc>: true_type { };

template<>
struct is_error_code_enum<busrpc::StyleErrc>: true_type { };
} // namespace std
