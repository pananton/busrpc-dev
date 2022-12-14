#pragma once

#include "entities/entity.h"
#include "entities/implemented_method.h"
#include "entities/invoked_method.h"
#include "entities/services.h"
#include "entities/struct.h"

#include <map>
#include <string>

/// \file service.h Service entity.

namespace busrpc {

class Services;
class ImplementedMethod;
class InvokedMethod;

/// Service entity.
class Service: public GeneralCompositeEntity {
public:
    using GeneralCompositeEntity::addStruct;
    using GeneralCompositeEntity::addEnum;

    /// Name of the documentation command, which sets service author.
    static constexpr const char* Author_Doc_Command = "author";

    /// Name of the documentation command, which sets service contact email.
    static constexpr const char* Email_Doc_Command = "author";

    /// Name of the documentation command, which sets service sources/documentation URL.
    static constexpr const char* Url_Doc_Command = "url";

    /// Service descriptor.
    const Struct* descriptor() const noexcept { return descriptor_; }

    /// Service config.
    const Struct* config() const noexcept { return config_; }

    /// Implemented methods ordered by method distinguished name.
    const std::map<std::string, const ImplementedMethod*>& implementedMethods() const noexcept
    {
        return implementedMethods_;
    }

    /// Invoked methods ordered by method distinguished name.
    const std::map<std::string, const InvokedMethod*>& invokedMethods() const noexcept { return invokedMethods_; }

    /// Service author.
    const std::string& author() const noexcept { return author_; }

    /// Service contact email.
    const std::string& email() const noexcept { return email_; }

    /// URL with service sources or some additional documentation.
    const std::string& url() const noexcept { return url_; }

    /// Entity representing all services.
    const Services* parent() const noexcept;

    /// Entity representing all services.
    Services* parent() noexcept;

protected:
    /// Create namespace entity.
    Service(CompositeEntity* services, const std::string& name);

private:
    friend class CompositeEntity;
    void onNestedEntityAdded(Entity* entity);
    void parseDocCommands();

    const Struct* descriptor_ = nullptr;
    const Struct* config_ = nullptr;
    std::map<std::string, const ImplementedMethod*> implementedMethods_;
    std::map<std::string, const InvokedMethod*> invokedMethods_;
    std::string author_;
    std::string email_;
    std::string url_;
};
} // namespace busrpc
