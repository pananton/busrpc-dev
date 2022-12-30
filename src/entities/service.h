#pragma once

#include "entities/entity.h"
#include "entities/implementation.h"
#include "entities/struct.h"

#include <map>
#include <optional>
#include <set>
#include <string>

/// \file service.h Service entity.

namespace busrpc {

class Implementation;
class Service;

/// Method implemented or invoked by the service.
/// \note This class and it's descendants do not represent a busrpc entity.
class ImportedMethod {
public:
    /// Imported method distinguished name.
    const std::string& dname() const noexcept { return dname_; }

    /// Imported method documentation.
    const EntityDocs& docs() const noexcept { return docs_; }

protected:
    /// Create imported method.
    ImportedMethod(std::string dname, EntityDocs docs): dname_(std::move(dname)), docs_(std::move(docs)) { }

    /// Virtual destructor.
    virtual ~ImportedMethod() = default;

private:
    std::string dname_;
    EntityDocs docs_;
};

/// Descendant order of imported methods distinguished names.
struct OrderImportedMethodsByDnameAsc {
    /// Indicates transparent comparator (allows to use \c std::string_view when searching for \ref ImportedMethod).
    using is_transparent = void;

    /// Return \c true if disntiguished name of \a lhs is less than distinguished name of \a rhs.
    bool operator()(const ImportedMethod& lhs, const ImportedMethod& rhs) const noexcept
    {
        return lhs.dname() < rhs.dname();
    }

    /// Return \c true if disntiguished name of \a lhs is less than \a rhs.
    bool operator()(const ImportedMethod& lhs, std::string_view dname) const noexcept { return lhs.dname() < dname; }

    /// Return \c true if \a lhs is less than distinguished name of \a rhs.
    bool operator()(std::string_view lhs, const ImportedMethod& rhs) const noexcept { return lhs < rhs.dname(); }
};

// clang-format off

/// Implemented/invoked method container.
template<typename T> requires std::is_base_of_v<ImportedMethod, T>
using ImportedMethodContainer = std::set<T, OrderImportedMethodsByDnameAsc>;

// clang-format on

/// Method implemented by the service.
class ImplementedMethod: public ImportedMethod {
public:
    /// Description of the accepted object identifier value.
    const std::optional<std::string>& acceptedObjectId() const noexcept { return acceptedObjectId_; }

    /// Descriptions of the accepted observable paremeter values ordered by the parameter name.
    const std::map<std::string, std::string>& acceptedParams() const noexcept { return acceptedParams_; }

private:
    friend class Service;
    ImplementedMethod(std::string dname, EntityDocs docs): ImportedMethod(std::move(dname), std::move(docs))
    {
        parseDocCommands();
    }

    void parseDocCommands();

    std::optional<std::string> acceptedObjectId_;
    std::map<std::string, std::string> acceptedParams_;
};

/// Method invoked by the service.
class InvokedMethod: public ImportedMethod {
private:
    friend class Service;
    InvokedMethod(std::string dname, EntityDocs docs): ImportedMethod(std::move(dname), std::move(docs)) { }
};

/// Service entity.
class Service: public GeneralCompositeEntity {
public:
    using GeneralCompositeEntity::addStruct;
    using GeneralCompositeEntity::addEnum;

    /// Entity representing API implementation.
    const Implementation* parent() const noexcept;

    /// Entity representing API implementation.
    Implementation* parent() noexcept;

    /// Service descriptor.
    const Struct* descriptor() const noexcept { return descriptor_; }

    /// Service config.
    const Struct* config() const noexcept { return config_; }

    /// Implemented methods ordered by method distinguished name.
    const ImportedMethodContainer<ImplementedMethod>& implementedMethods() const noexcept
    {
        return implementedMethods_;
    }

    /// Invoked methods ordered by method distinguished name.
    const ImportedMethodContainer<InvokedMethod>& invokedMethods() const noexcept { return invokedMethods_; }

    /// Service author.
    const std::string& author() const noexcept { return author_; }

    /// Service contact email.
    const std::string& email() const noexcept { return email_; }

    /// URL with service sources or some additional documentation.
    const std::string& url() const noexcept { return url_; }

protected:
    /// Create namespace entity.
    Service(CompositeEntity* services, const std::string& name);

private:
    friend class CompositeEntity;
    void onNestedEntityAdded(Entity* entity);
    void parseDocCommands();

    const Struct* descriptor_ = nullptr;
    const Struct* config_ = nullptr;
    ImportedMethodContainer<ImplementedMethod> implementedMethods_;
    ImportedMethodContainer<InvokedMethod> invokedMethods_;
    std::string author_;
    std::string email_;
    std::string url_;
};
} // namespace busrpc
