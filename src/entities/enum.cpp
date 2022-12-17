#include "entities/enum.h"
#include "entities/struct.h"

#include <cassert>

namespace busrpc {

Enum::Enum(CompositeEntity* parent,
           const std::string& name,
           const std::string& filename,
           EntityDocs docs):
    CompositeEntity(parent, EntityTypeId::Enum, name, std::move(docs)),
    package_{},
    file_{},
    constants_{}
{
    assert(this->parent());

    if (this->parent()->type() == EntityTypeId::Struct) {
        package_ = static_cast<const Struct*>(this->parent())->package();
        file_ = static_cast<const Struct*>(this->parent())->file();
    } else {
        package_ = this->parent()->dname();
        std::filesystem::path file(filename);

        if (!file.has_filename() || file.has_parent_path()) {
            throw entity_error(EntityTypeId::Enum,
                               dname(),
                               "unexpected filename '" + filename +
                                   "' (either invalid or contains directory components)");
        }

        file_ = dir() / file.filename();
    }
}

Constant* Enum::addConstant(const std::string& name, int32_t value, EntityDocs docs)
{
    Constant* constant = addNestedEntity<Constant>(name, value, std::move(docs));
    constants_.insert(constant);
    return constant;
}
} // namespace busrpc