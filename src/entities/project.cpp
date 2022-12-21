#include "entities/project.h"

#include <cassert>

namespace busrpc {

namespace {
class SpecErrorCategory: public std::error_category {
public:
    const char* name() const noexcept override { return "specification errors"; }

    std::string message(int code) const override
    {
        using enum SpecErrc;

        switch (static_cast<SpecErrc>(code)) {
        case Missing_Api: return "Project does not define an API";
        case Missing_Builtin: return "Busrpc built-in type could not be found";
        case Nonconforming_Builtin: return "Busrpc built-in type does not conform with specification";
        case Missing_Descriptor: return "Descriptor could not be found";
        case Not_Static_Method: return "Method is not static";
        case Not_Encodable_Type: return "Type is not encodable";
        case Not_Accessible_Type: return "Type is not accessible in the current scope";
        default: return "Unknown error";
        }
    }
};

class SpecWarnCategory: public std::error_category {
public:
    const char* name() const noexcept override { return "specification warnings"; }

    std::string message(int code) const override
    {
        using enum SpecWarn;

        switch (static_cast<SpecWarn>(code)) {
        case Unexpected_Nested_Entity: return "Entity contains unexpected nested entity";
        default: return "Unknown error";
        }
    }
};

class DocErrorCategory: public std::error_category {
public:
    const char* name() const noexcept override { return "documentation errors"; }

    std::string message(int code) const override
    {
        using enum DocErrc;

        switch (static_cast<DocErrc>(code)) {
        case Undocumented_Entity: return "Entity is not documented";
        case Unknown_Doc_Command: return "Unknown documentation command";
        default: return "Unknown error";
        }
    }
};

class StyleErrorCategory: public std::error_category {
public:
    const char* name() const noexcept override { return "style errors"; }

    std::string message(int code) const override
    {
        using enum StyleErrc;

        switch (static_cast<StyleErrc>(code)) {
        case Invalid_Name_Format: return "Entity name format is invalid";
        default: return "Unknown error";
        }
    }
};

} // namespace

Project::Project(std::filesystem::path root):
    CompositeEntity(nullptr, EntityTypeId::Project, Project_Entity_Name, {{Project_Entity_Description}, {}}),
    root_(std::move(root))
{
    setNestedEntityAddedCallback([this](Entity* entity) { onNestedEntityAdded(entity); });
    entityDirectory_[name()] = this;
}

Api* Project::addApi()
{
    Api* api = addNestedEntity<Api>();
    api_ = api;
    return api;
}

Services* Project::addServices()
{
    Services* services = addNestedEntity<Services>();
    services_ = services;
    return services;
}

const Entity* Project::find(const std::string& dname) const noexcept
{
    std::string prefix = Project_Entity_Name;
    prefix.append(1, '.');
    std::string normalized;

    if (!dname.starts_with(prefix)) {
        normalized = prefix;

        if (!dname.empty() && dname != Project_Entity_Name) {
            normalized.append(dname);
        } else {
            normalized.erase(normalized.size() - 1);
        }
    } else {
        normalized = dname;
    }

    auto it = entityDirectory_.find(normalized);
    return it != entityDirectory_.end() ? it->second : nullptr;
}

ErrorCollector Project::check() const noexcept
{
    ErrorCollector ecol;
    check(ecol);
    return ecol;
}

void Project::check(ErrorCollector& ecol) const noexcept
{
    if (!api_) {
        ecol.add(SpecErrc::Missing_Api);
        return;
    }

    checkApi(api_, ecol);
}

void Project::onNestedEntityAdded(Entity* entity)
{
    auto isAdded = entityDirectory_.emplace(entity->dname(), entity).second;
    assert(isAdded);
}

void Project::checkApi(const Api*, ErrorCollector&) const noexcept { }

void Project::checkServices(const Services*, ErrorCollector&) const noexcept { }

const std::error_category& spec_error_category()
{
    static const SpecErrorCategory category;
    return category;
}

std::error_code make_error_code(SpecErrc e)
{
    return {static_cast<int>(e), spec_error_category()};
}

const std::error_category& spec_warn_category()
{
    static const SpecWarnCategory category;
    return category;
}

std::error_code make_error_code(SpecWarn e)
{
    return {static_cast<int>(e), spec_warn_category()};
}

const std::error_category& doc_error_category()
{
    static const DocErrorCategory category;
    return category;
}

std::error_code make_error_code(DocErrc e)
{
    return {static_cast<int>(e), doc_error_category()};
}

const std::error_category& style_error_category()
{
    static const StyleErrorCategory category;
    return category;
}

std::error_code make_error_code(StyleErrc e)
{
    return {static_cast<int>(e), style_error_category()};
}
} // namespace busrpc
