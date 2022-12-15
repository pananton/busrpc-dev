#include "entities/implemented_method.h"
#include "utils.h"

#include <cassert>

namespace busrpc {

ImplementedMethod::ImplementedMethod(CompositeEntity* service, const std::string& name, EntityDocs docs):
    Entity(service, EntityTypeId::Implemented_Method, name, std::move(docs))
{
    assert(dynamic_cast<Service*>(this->parent()));
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

void ImplementedMethod::parseDocCommands()
{
    auto acceptedValues = docs().commands().find(doc_cmd::Accepted_Value);

    if (acceptedValues == docs().commands().end()) {
        return;
    }

    for (const auto& cmdValue: acceptedValues->second) {
        std::string paramName;
        std::string paramValue;
        auto paramNameEnd = cmdValue.find_first_of(" \t");

        if (paramNameEnd != std::string::npos) {
            paramName = cmdValue.substr(0, paramNameEnd);
            paramValue = TrimString(cmdValue.substr(paramNameEnd + 1));
        } else {
            paramName = cmdValue;
            paramValue = "";
        }

        if (paramName == "@object_id") {
            acceptedObjectId_ = paramValue;
        } else {
            acceptedParams_[paramName] = paramValue;
        }
    }
}
} // namespace busrpc
