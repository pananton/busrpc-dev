#include "generators/generator.h"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <ostream>

/// \file json_generator.h Generator, which outputs busrpc project documentation in the JSON format.

namespace busrpc {

/// Generator, which outputs a single JSON document containint busrpc project documentation.
class JsonGenerator: public DocGenerator {
public:
    /// Create JSON generator, which outputs generated JSON document to \a out.
    /// \warning Stream \a out should outlive generator.
    JsonGenerator(std::ostream& out): out_(out) { }

    /// Generate and output JSON document containing busrpc project documentation.
    void generate(const Project* project) const override;

private:
    std::ostream& out_;
};

/// Convert \ref Project to json.
void to_json(nlohmann::json& obj, const Project& project);

/// Convert \ref Api to json.
void to_json(nlohmann::json& obj, const Api& api);

/// Convert \ref Namespace to json.
void to_json(nlohmann::json& obj, const Namespace& ns);

/// Convert \ref Class to json.
void to_json(nlohmann::json& obj, const Class& cls);

/// Convert \ref Method to json.
void to_json(nlohmann::json& obj, const Method& method);

/// Convert \ref Services to json.
void to_json(nlohmann::json& obj, const Services& services);

/// Convert \ref Service to json.
void to_json(nlohmann::json& obj, const Service& service);

/// Convert \ref ImplementedMethod to json.
void to_json(nlohmann::json& obj, const ImplementedMethod& implMethod);

/// Convert \ref InvokedMethod to json.
void to_json(nlohmann::json& obj, const InvokedMethod& invkMethod);

/// Convert \ref Struct to json.
void to_json(nlohmann::json& obj, const Struct& structure);

/// Convert \ref Field to json.
void to_json(nlohmann::json& obj, const Field& field);

/// Convert \ref Enum to json.
void to_json(nlohmann::json& obj, const Enum& enumeration);

/// Convert \ref Constant to json.
void to_json(nlohmann::json& obj, const Constant& constant);

/// Convert \ref EntityDocs to json.
void to_json(nlohmann::json& obj, const EntityDocs& docs);
} // namespace busrpc
