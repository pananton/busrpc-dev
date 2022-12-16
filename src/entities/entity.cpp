#include "entities/entity.h"
#include "entities/enum.h"
#include "entities/struct.h"
#include "utils.h"

namespace busrpc {

EntityDocs::EntityDocs(std::vector<std::string> description, std::map<std::string, std::vector<std::string>> commands):
    description_(std::move(description)),
    brief_{},
    commands_(std::move(commands))
{
    if (!description_.empty()) {
        brief_ = description_[0];
    }

    for (auto it = commands_.begin(); it != commands_.end(); ++it) {
        if (it->second.empty()) {
            it->second.emplace_back("");
        }
    }
}

EntityDocs::EntityDocs(const std::string& blockComment)
{
    constexpr const char* whitespace = " \t";
    auto lines = SplitString(blockComment);

    for (const auto& line: lines) {
        auto commandStartPos = line.find_first_not_of(whitespace);

        if (commandStartPos == std::string::npos || line[commandStartPos] != '\\') {
            description_.push_back(line);
        } else {
            // parsing documentation command, which consists of name and value

            auto nameStartPos = commandStartPos + 1;
            auto nameEndPos = line.find_first_of(whitespace, nameStartPos);
            size_t nameLength = std::string::npos;

            if (nameEndPos != std::string::npos) {
                nameLength = nameEndPos - nameStartPos;
            }

            auto it = commands_.emplace(line.substr(nameStartPos, nameLength), std::vector<std::string>{}).first;
            it->second.emplace_back(nameEndPos != std::string::npos ? TrimString(line.substr(nameEndPos)) : "");
        }
    }

    if (!description_.empty()) {
        brief_ = description_.front();
    }
}

Entity::Entity(CompositeEntity* parent, EntityTypeId type, const std::string& name, EntityDocs docs):
    parent_(parent),
    type_(type),
    name_(name),
    dir_{},
    docs_(std::move(docs))
{
    if (!IsValidEntityName(name)) {
        throw entity_error(type_, name_, "invalid entity name");
    }

    if (parent_) {
        dir_ = parent_->dir();

        switch (type_) {
        case EntityTypeId::Struct:
        case EntityTypeId::Field:
        case EntityTypeId::Enum:
        case EntityTypeId::Constant:
        case EntityTypeId::Implemented_Method:
        case EntityTypeId::Invoked_Method: break;
        default: dir_ /= name_;
        }
    }
}

DistinguishedEntity::DistinguishedEntity(CompositeEntity* parent,
                                         EntityTypeId type,
                                         const std::string& name,
                                         EntityDocs docs):
    Entity(parent, type, name, std::move(docs)),
    dname_{}
{
    if (parent) {
        dname_ = parent->dname() + ".";
    }

    dname_.append(this->name());
}

Struct* GeneralCompositeEntity::addStruct(const std::string& name,
                                          const std::string& filename,
                                          StructFlags flags,
                                          EntityDocs docs)
{
    auto ptr = addNestedEntity<Struct>(name, filename, flags, std::move(docs));
    structs_[ptr->name()] = ptr;
    return ptr;
}

Enum* GeneralCompositeEntity::addEnum(const std::string& name, const std::string& filename, EntityDocs docs)
{
    auto ptr = addNestedEntity<Enum>(name, filename, std::move(docs));
    enums_[ptr->name()] = ptr;
    return ptr;
}
} // namespace busrpc
