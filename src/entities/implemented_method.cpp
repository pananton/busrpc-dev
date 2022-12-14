#include "entities/implemented_method.h"
#include "utils.h"

#include <cassert>

namespace busrpc {

ImplementedMethod::ImplementedMethod(CompositeEntity* service,
                                     const std::string& name,
                                     const std::vector<std::string>& description,
                                     const std::string& briefDescription,
                                     const std::multimap<std::string, std::string>& docCommands):
    Entity(service, EntityTypeId::Implemented_Method, name, "")
{
    assert(dynamic_cast<Service*>(this->parent()));
    setDocumentation(description, briefDescription, docCommands);
    parseDocCommands();
}

const Service* ImplementedMethod::parent() const noexcept
{
    return static_cast<const Service*>(Entity::parent());
}

Service* ImplementedMethod::parent() noexcept
{
    return static_cast<Service*>(Entity::parent());
}

void ImplementedMethod::parseDocCommands() {
    for (auto it = docCommands().find(Accept_Doc_Command); it != docCommands().upper_bound(Accept_Doc_Command); ++it) {
        std::string name;
        std::string value;
        auto nameEnd = it->second.find_first_of(" \t");

        if (nameEnd != std::string::npos) {
            name = it->second.substr(0, nameEnd);
            value = TrimString(it->second.substr(nameEnd + 1));
        } else {
            name = it->second;
            value = "";
        }

        if (name == "@object_id") {
            acceptedObjectId_ = value;
        } else {
            acceptedParams_[name] = value;
        }
    }
}
} // namespace busrpc
