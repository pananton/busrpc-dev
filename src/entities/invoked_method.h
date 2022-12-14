#pragma once

#include "entities/entity.h"
#include "entities/service.h"

#include <map>
#include <string>
#include <vector>

/// \file invoked_method.h Invoked method entity.

namespace busrpc {

class Service;

/// Entity representing method invoked by a service.
/// \note Name of the \c InvokedMethod entity is the distinguished name of the corresponding \ref Method entity.
class InvokedMethod: public Entity {
public:
    /// Service, which invokes method.
    const Service* parent() const noexcept;

    /// Service, which invokes method.
    Service* parent() noexcept;

protected:
    /// Create invoked method entity.
    InvokedMethod(CompositeEntity* service,
                  const std::string& name,
                  const std::vector<std::string>& description,
                  const std::string& briefDescription,
                  const std::multimap<std::string, std::string>& docCommands);

private:
    friend class CompositeEntity;
};
} // namespace busrpc
