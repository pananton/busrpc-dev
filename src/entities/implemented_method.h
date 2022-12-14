#pragma once

#include "entities/entity.h"
#include "entities/service.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

/// \file implemented_method.h Implemented method entity.

namespace busrpc {

class Service;

/// Entity representing method implemented by a service.
/// \note Name of the \c ImplementedEntity entity is the distinguished name of the corresponding \ref Method entity.
class ImplementedMethod: public Entity {
public:
    /// Name of the documentation command, which specifies accepted object identifier and observable parameter values.
    static constexpr const char* Accept_Doc_Command = "accept";

    /// Description of accepted object identifier value.
    const std::optional<std::string>& acceptedObjectId() const noexcept { return acceptedObjectId_; }

    /// Descriptions of accepted observable paremeter values ordered by parameter name.
    const std::map<std::string, std::string>& acceptedParams() const noexcept { return acceptedParams_; }

    /// Service, which implements method.
    const Service* parent() const noexcept;

    /// Service, which implements method.
    Service* parent() noexcept;

protected:
    /// Create implemented method entity.
    ImplementedMethod(CompositeEntity* service,
                      const std::string& name,
                      const std::vector<std::string>& description,
                      const std::string& briefDescription,
                      const std::multimap<std::string, std::string>& docCommands);

private:
    friend class CompositeEntity;
    void parseDocCommands();

    std::optional<std::string> acceptedObjectId_;
    std::map<std::string, std::string> acceptedParams_;
};
} // namespace busrpc
