#include "entities/entity.h"
#include "entities/enum.h"
#include "entities/struct.h"
#include "utils.h"

namespace busrpc {

Entity::Entity(CompositeEntity* parent, EntityTypeId type, const std::string& name, const std::string& blockComment):
    parent_(parent),
    type_(type),
    name_(name),
    dir_{},
    description_{},
    docCommands_{}
{
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

    parseBlockComment(blockComment);
}

void Entity::parseBlockComment(const std::string& blockComment)
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

            docCommands_.emplace(line.substr(nameStartPos, nameLength),
                                 nameEndPos != std::string::npos ? TrimString(line.substr(nameEndPos)) : "");
        }
    }

    if (!description_.empty()) {
        briefDescription_ = description_.front();
    }
}

DistinguishedEntity::DistinguishedEntity(CompositeEntity* parent,
                                         EntityTypeId type,
                                         const std::string& name,
                                         const std::string& blockComment):
    Entity(parent, type, name, blockComment),
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
                                          const std::string& blockComment)
{
    auto ptr = addNestedEntity<Struct>(name, filename, flags, blockComment);
    structs_[ptr->name()] = ptr;
    return ptr;
}

Enum* GeneralCompositeEntity::addEnum(const std::string& name,
                                      const std::string& filename,
                                      const std::string& blockComment)
{
    auto ptr = addNestedEntity<Enum>(name, filename, blockComment);
    enums_[ptr->name()] = ptr;
    return ptr;
}
} // namespace busrpc
