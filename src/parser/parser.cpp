#include "parser/parser.h"
#include "protobuf_error_collector.h"
#include "utils.h"

#pragma warning(push)
#pragma warning(disable: 4100)
#pragma warning(disable: 4251)
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#pragma warning(pop)

#include <fstream>

namespace protobuf = google::protobuf;

namespace busrpc {

namespace {

class ParserErrorCategory: public std::error_category {
public:
    const char* name() const noexcept override { return "parser error"; }

    std::string message(int code) const override
    {
        using enum ParserErrc;

        switch (static_cast<ParserErrc>(code)) {
        case Invalid_Project_Dir: return "Directory does not represent a valid busrpc project directory.";
        case Read_Failed: return "Failed to read file";
        case Protobuf_Error: return "Protobuf error";
        default: return "Unknown error";
        }
    }
};

Enum* CreateEnum(GeneralCompositeEntity* entity,
                 const std::string& name,
                 const std::string& filename,
                 const std::string& blockComment)
{
    switch (entity->type()) {
    case EntityTypeId::Project: return static_cast<Project*>(entity)->addEnum(name, filename, EntityDocs(blockComment));
    case EntityTypeId::Api: return static_cast<Api*>(entity)->addEnum(name, filename, EntityDocs(blockComment));
    case EntityTypeId::Namespace:
        return static_cast<Namespace*>(entity)->addEnum(name, filename, EntityDocs(blockComment));
    case EntityTypeId::Class: return static_cast<Class*>(entity)->addEnum(name, filename, EntityDocs(blockComment));
    case EntityTypeId::Method: return static_cast<Method*>(entity)->addEnum(name, filename, EntityDocs(blockComment));
    case EntityTypeId::Implementation:
        return static_cast<Implementation*>(entity)->addEnum(name, filename, EntityDocs(blockComment));
    case EntityTypeId::Service: return static_cast<Service*>(entity)->addEnum(name, filename, EntityDocs(blockComment));
    case EntityTypeId::Struct: return static_cast<Struct*>(entity)->addEnum(name, EntityDocs(blockComment));
    default: return nullptr;
    }
}

Struct* CreateStruct(GeneralCompositeEntity* entity,
                     const std::string& name,
                     const std::string& filename,
                     StructFlags flags,
                     const std::string& blockComment)
{
    switch (entity->type()) {
    case EntityTypeId::Project:
        return static_cast<Project*>(entity)->addStruct(name, filename, flags, EntityDocs(blockComment));
    case EntityTypeId::Api:
        return static_cast<Api*>(entity)->addStruct(name, filename, flags, EntityDocs(blockComment));
    case EntityTypeId::Namespace:
        return static_cast<Namespace*>(entity)->addStruct(name, filename, flags, EntityDocs(blockComment));
    case EntityTypeId::Class:
        return static_cast<Class*>(entity)->addStruct(name, filename, flags, EntityDocs(blockComment));
    case EntityTypeId::Method:
        return static_cast<Method*>(entity)->addStruct(name, filename, flags, EntityDocs(blockComment));
    case EntityTypeId::Implementation:
        return static_cast<Implementation*>(entity)->addStruct(name, filename, flags, EntityDocs(blockComment));
    case EntityTypeId::Service:
        return static_cast<Service*>(entity)->addStruct(name, filename, flags, EntityDocs(blockComment));
    case EntityTypeId::Struct: return static_cast<Struct*>(entity)->addStruct(name, flags, EntityDocs(blockComment));
    default: return nullptr;
    }
}

std::optional<FieldTypeId> ToBusrpcType(int protobufType)
{
    using enum FieldTypeId;

    switch (protobufType) {
    case protobuf::FieldDescriptor::TYPE_BOOL: return Bool;
    case protobuf::FieldDescriptor::TYPE_INT32: return Int32;
    case protobuf::FieldDescriptor::TYPE_SINT32: return Sint32;
    case protobuf::FieldDescriptor::TYPE_SFIXED32: return Sfixed32;
    case protobuf::FieldDescriptor::TYPE_UINT32: return Uint32;
    case protobuf::FieldDescriptor::TYPE_FIXED32: return Fixed32;
    case protobuf::FieldDescriptor::TYPE_INT64: return Int64;
    case protobuf::FieldDescriptor::TYPE_SINT64: return Sint64;
    case protobuf::FieldDescriptor::TYPE_SFIXED64: return Sfixed64;
    case protobuf::FieldDescriptor::TYPE_UINT64: return Uint64;
    case protobuf::FieldDescriptor::TYPE_FIXED64: return Fixed64;
    case protobuf::FieldDescriptor::TYPE_FLOAT: return Float;
    case protobuf::FieldDescriptor::TYPE_DOUBLE: return Double;
    case protobuf::FieldDescriptor::TYPE_STRING: return String;
    case protobuf::FieldDescriptor::TYPE_BYTES: return Bytes;
    case protobuf::FieldDescriptor::TYPE_ENUM: return Enum;
    case protobuf::FieldDescriptor::TYPE_MESSAGE: return Message;
    default: return std::nullopt;
    }
}

template<typename TDescriptorProto>
const TDescriptorProto* FindDescriptorProto(const protobuf::RepeatedPtrField<TDescriptorProto>& descriptors,
                                            const std::string& name)
{
    for (int i = 0; i < descriptors.size(); ++i) {
        if (descriptors[i].name() == name) {
            return &descriptors[i];
        }
    }

    return nullptr;
}
} // namespace

std::pair<ProjectPtr, ErrorCollector> Parser::parse(std::vector<const std::error_category*> ignoredCategories) const
{
    SeverityOrder orderFunc = [](std::error_code lhs, std::error_code rhs) {
        if (lhs.category() == rhs.category()) {
            return false;
        }

        if (rhs.category() == parser_error_category() ||

            (rhs.category() == spec_error_category() && lhs.category() != parser_error_category()) ||

            (rhs.category() == spec_warn_category() && lhs.category() != parser_error_category() &&
             lhs.category() != spec_error_category()) ||

            (rhs.category() == doc_warn_category() && lhs.category() == style_warn_category())) {

            return true;
        }

        return false;
    };

    ErrorCollector ecol(ParserErrc::Protobuf_Error, std::move(orderFunc), std::move(ignoredCategories));
    auto projectPtr = parse(ecol);
    return std::make_pair(projectPtr, std::move(ecol));
}

ProjectPtr Parser::parse(ErrorCollector& ecol) const
{
    auto projectPtr = std::make_shared<Project>(projectDir_);
    ProtobufErrorCollector protobufCollector(ecol, ParserErrc::Protobuf_Error);
    std::filesystem::path projectPath;
    std::filesystem::path protobufPath;

    try {
        InitCanonicalPathToExistingDirectory(projectPath, projectDir_.string());

        if (!protobufRoot_.empty()) {
            InitCanonicalPathToExistingDirectory(protobufPath, protobufRoot_.string());
        }
    } catch (const std::filesystem::filesystem_error&) { }

    if (projectPath.empty() || !std::filesystem::is_regular_file(projectPath / Busrpc_Builtin_File)) {
        ecol.add(ParserErrc::Invalid_Project_Dir, std::make_pair("dir", projectDir_));
        return projectPtr;
    }

    protobuf::compiler::DiskSourceTree sourceTree;
    sourceTree.MapPath("", projectPath.generic_string());

    if (!protobufPath.empty()) {
        sourceTree.MapPath("", protobufPath.generic_string());
    }

#ifndef _WIN32
    sourceTree.MapPath("", "/usr/include");
    sourceTree.MapPath("", "/usr/local/include");
#endif

    protobuf::compiler::Importer importer(
        &sourceTree, ecol.getProtobufCollector() ? ecol.getProtobufCollector() : &protobufCollector);

    parseDir(importer, projectPtr.get(), ecol);

    if (!ecol.majorError() || ecol.majorError()->code.category() != parser_error_category()) {
        projectPtr->check(ecol);
    }

    return projectPtr;
}

GeneralCompositeEntity* Parser::visitSubdirectory(GeneralCompositeEntity* parent,
                                                  ErrorCollector& ecol,
                                                  const std::string& subdirName) const
{
    switch (parent->type()) {
    case EntityTypeId::Project:
        {
            if (subdirName == Api_Entity_Name) {
                return static_cast<Project*>(parent)->addApi();
            } else if (subdirName == Implementation_Entity_Name) {
                return static_cast<Project*>(parent)->addImplementation();
            }

            break;
        }
    case EntityTypeId::Api: return static_cast<Api*>(parent)->addNamespace(subdirName);
    case EntityTypeId::Namespace: return static_cast<Namespace*>(parent)->addClass(subdirName);
    case EntityTypeId::Class: return static_cast<Class*>(parent)->addMethod(subdirName);
    case EntityTypeId::Implementation: return static_cast<Implementation*>(parent)->addService(subdirName);
    default: break;
    }

    // May occur only when subdirectory is not part of busrpc directory layout and should be ignored

    ecol.add(SpecWarn::Unexpected_Nested_Entity,
             std::make_pair("dir", parent->dir() / subdirName),
             "directory is not part of the specification and is ignored");
    return nullptr;
}

void Parser::parseDir(google::protobuf::compiler::Importer& importer,
                      GeneralCompositeEntity* entity,
                      ErrorCollector& ecol) const
{
    std::error_code ec;
    std::filesystem::directory_iterator dirIt(projectDir_ / entity->dir(), ec);
    std::set<std::string> subdirs;

    while (dirIt != std::filesystem::directory_iterator() && !ec) {
        if (dirIt->is_regular_file() && dirIt->path().extension() == ".proto") {
            std::string relPath = (entity->dir() / dirIt->path().filename()).generic_string();
            const protobuf::FileDescriptor* fileDesc = importer.Import(relPath.c_str());
            protobuf::FileDescriptorProto fileDescProto;
            bool hasErrors = true;

            if (fileDesc) {
                std::ifstream file((projectDir_ / relPath).string().c_str());

                if (file.is_open()) {
                    protobuf::io::IstreamInputStream in(&file);
                    protobuf::io::Tokenizer tokenizer(&in, nullptr);
                    protobuf::compiler::Parser parser;
                    hasErrors = !parser.Parse(&tokenizer, &fileDescProto);
                } else {
                    ecol.add(ParserErrc::Read_Failed, std::make_pair("file", relPath), "failed to open file");
                }
            }

            if (!hasErrors) {
                // any error should be already added to collector by the importer object
                parseFile(fileDesc, &fileDescProto, entity, ecol);
            }
        } else if (dirIt->is_directory()) {
            subdirs.insert((--(dirIt->path().end()))->string());
        }

        dirIt.increment(ec);
    }

    for (const auto& subdir: subdirs) {
        GeneralCompositeEntity* nestedEntity = nullptr;

        try {
            nestedEntity = visitSubdirectory(entity, ecol, subdir);
        } catch (const name_conflict_error&) {
            ecol.add(SpecErrc::Multiple_Definitions,
                     std::make_pair(GetEntityTypeIdStr(entity->type()), entity->dname()),
                     "nested entity '" + subdir + "'is defined more than once");
        } catch (const entity_error& e) {
            ecol.add(SpecErrc::Invalid_Entity,
                     std::make_pair(GetEntityTypeIdStr(entity->type()), entity->dname()),
                     "failed to create nested entity '" + subdir + "', exception caught (" + e.what() + ")");
        }

        if (nestedEntity) {
            parseDir(importer, nestedEntity, ecol);
        }
    }

    if (ec) {
        ecol.add(
            ParserErrc::Read_Failed, std::make_pair("dir", entity->dir()), "can't iterate through directory content");
        return;
    }
}

void Parser::parseFile(const protobuf::FileDescriptor* fileDesc,
                       const google::protobuf::FileDescriptorProto* fileDescProto,
                       GeneralCompositeEntity* entity,
                       ErrorCollector& ecol) const
{
    if (fileDesc->package() != entity->dname()) {
        ecol.add(SpecErrc::Unexpected_Package,
                 std::make_pair("file", fileDesc->name()),
                 "file content should be placed in '" + entity->dname() + "' package");
        return;
    }

    std::string filename = std::filesystem::path(fileDesc->name()).filename().string();

    for (int i = 0; i < fileDesc->enum_type_count(); ++i) {
        auto enumDesc = fileDesc->enum_type(i);

        try {
            addEnum(entity, fileDesc->enum_type(i), filename);
        } catch (const name_conflict_error&) {
            ecol.add(SpecErrc::Multiple_Definitions,
                     std::make_pair(GetEntityTypeIdStr(entity->type()), entity->dname()),
                     "nested entity '" + enumDesc->name() + "'is defined more than once");
        } catch (const entity_error& e) {
            ecol.add(SpecErrc::Invalid_Entity,
                     std::make_pair(GetEntityTypeIdStr(entity->type()), entity->dname()),
                     "failed to create nested entity '" + enumDesc->name() + "', exception caught (" + e.what() + ")");
        }
    }

    for (int i = 0; i < fileDesc->message_type_count(); ++i) {
        auto structDesc = fileDesc->message_type(i);
        const protobuf::DescriptorProto* structDescProto =
            FindDescriptorProto<protobuf::DescriptorProto>(fileDescProto->message_type(), structDesc->name());

        assert(structDescProto);

        try {
            addStruct(entity, structDesc, structDescProto, filename);
        } catch (const name_conflict_error&) {
            ecol.add(SpecErrc::Multiple_Definitions,
                     std::make_pair(GetEntityTypeIdStr(entity->type()), entity->dname()),
                     "nested entity '" + structDesc->name() + "'is defined more than once");
        } catch (const entity_error& e) {
            ecol.add(SpecErrc::Invalid_Entity,
                     std::make_pair(GetEntityTypeIdStr(entity->type()), entity->dname()),
                     "failed to create nested entity '" + structDesc->name() + "', exception caught (" + e.what() +
                         ")");
        }
    }
}

void Parser::addEnum(GeneralCompositeEntity* entity,
                     const google::protobuf::EnumDescriptor* desc,
                     const std::string& filename) const
{
    google::protobuf::SourceLocation source;
    desc->GetSourceLocation(&source);
    Enum* enumeration = CreateEnum(entity, desc->name(), filename, source.leading_comments);

    assert(enumeration);
    initEnum(enumeration, desc);
}

void Parser::initEnum(Enum* enumeration, const google::protobuf::EnumDescriptor* desc) const
{
    for (int i = 0; i < desc->value_count(); ++i) {
        google::protobuf::SourceLocation source;
        desc->value(i)->GetSourceLocation(&source);
        enumeration->addConstant(desc->value(i)->name(), desc->value(i)->number(), EntityDocs(source.leading_comments));
    }
}

void Parser::addStruct(GeneralCompositeEntity* entity,
                       const google::protobuf::Descriptor* desc,
                       const google::protobuf::DescriptorProto* descProto,
                       const std::string& filename) const
{
    protobuf::SourceLocation source;
    desc->GetSourceLocation(&source);
    StructFlags flags = StructFlags::None;

    for (int i = 0; i < descProto->options().uninterpreted_option_size(); ++i) {
        const auto& opt = descProto->options().uninterpreted_option(i);

        if (opt.name().size() == 1 && opt.name(0).name_part() == Message_Option_Hashed && opt.has_identifier_value()) {
            if (opt.identifier_value() == "true") {
                flags |= StructFlags::Hashed;
            }

            break;
        }
    }

    Struct* structure = CreateStruct(entity, desc->name(), filename, flags, source.leading_comments);
    initStruct(structure, desc, descProto);
}

void Parser::initStruct(Struct* structure,
                        const google::protobuf::Descriptor* desc,
                        const google::protobuf::DescriptorProto* descProto) const
{
    for (int i = 0; i < desc->field_count(); ++i) {
        auto fieldDesc = desc->field(i);
        const protobuf::FieldDescriptorProto* fieldDescProto =
            FindDescriptorProto<protobuf::FieldDescriptorProto>(descProto->field(), desc->field(i)->name());

        assert(fieldDescProto);
        addField(structure, fieldDesc, fieldDescProto);
    }

    for (int i = 0; i < desc->enum_type_count(); ++i) {
        addEnum(structure, desc->enum_type(i));
    }

    for (int i = 0; i < desc->nested_type_count(); ++i) {
        auto nestedDesc = desc->nested_type(i);

        if (!nestedDesc->options().map_entry()) {
            const protobuf::DescriptorProto* nestedDescProto =
                FindDescriptorProto<protobuf::DescriptorProto>(descProto->nested_type(), nestedDesc->name());
            assert(nestedDescProto);
            addStruct(structure, nestedDesc, nestedDescProto);
        }
    }
}

void Parser::addField(Struct* structure,
                      const google::protobuf::FieldDescriptor* desc,
                      const google::protobuf::FieldDescriptorProto* descProto) const
{
    google::protobuf::SourceLocation source;
    desc->GetSourceLocation(&source);
    FieldFlags flags = FieldFlags::None;
    std::string defaultValue;

    for (int i = 0; i < descProto->options().uninterpreted_option_size(); ++i) {
        const auto& opt = descProto->options().uninterpreted_option(i);

        if (opt.name().size() == 1 && opt.name(0).name_part() == Field_Option_Observable &&
            opt.has_identifier_value()) {

            if (opt.identifier_value() == "true") {
                flags |= FieldFlags::Observable;
            }
        } else if (opt.name().size() == 1 && opt.name(0).name_part() == Field_Option_Hashed &&
                   opt.has_identifier_value()) {

            if (opt.identifier_value() == "true") {
                flags |= FieldFlags::Hashed;
            }
        } else if (opt.name().size() == 1 && opt.name(0).name_part() == Field_Option_Default_Value &&
                   opt.has_string_value()) {

            defaultValue = opt.string_value();
        }
    }

    if (desc->has_optional_keyword()) {
        flags |= FieldFlags::Optional;
    }

    if (desc->is_repeated()) {
        flags |= FieldFlags::Repeated;
    }

    auto fieldType = ToBusrpcType(desc->type());
    assert(fieldType);

    if (IsScalarFieldType(*fieldType)) {
        structure->addScalarField(desc->name(),
                                  desc->number(),
                                  *fieldType,
                                  flags,
                                  desc->real_containing_oneof() ? desc->real_containing_oneof()->name() : "",
                                  defaultValue,
                                  EntityDocs(source.leading_comments));
    } else if (*fieldType == FieldTypeId::Message) {
        if (!desc->is_map()) {
            structure->addStructField(desc->name(),
                                      desc->number(),
                                      desc->message_type()->full_name(),
                                      flags,
                                      desc->real_containing_oneof() ? desc->real_containing_oneof()->name() : "",
                                      EntityDocs(source.leading_comments));
        } else {
            auto keyType = ToBusrpcType(desc->message_type()->map_key()->type());
            assert(keyType && IsScalarFieldType(*keyType));

            auto valueType = ToBusrpcType(desc->message_type()->map_value()->type());
            assert(valueType);

            std::string valueTypeName =
                IsScalarFieldType(*valueType) ? "" : desc->message_type()->map_value()->message_type()->full_name();

            structure->addMapField(
                desc->name(), desc->number(), *keyType, *valueType, valueTypeName, EntityDocs(source.leading_comments));
        }
    } else {
        structure->addEnumField(desc->name(),
                                desc->number(),
                                desc->enum_type()->full_name(),
                                flags,
                                desc->real_containing_oneof() ? desc->real_containing_oneof()->name() : "",
                                EntityDocs(source.leading_comments));
    }
}

const std::error_category& parser_error_category()
{
    static const ParserErrorCategory category;
    return category;
}

std::error_code make_error_code(ParserErrc e)
{
    return {static_cast<int>(e), parser_error_category()};
}
} // namespace busrpc
