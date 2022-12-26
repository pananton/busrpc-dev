#pragma once

#include "entities/project.h"
#include "error_collector.h"

#include <filesystem>
#include <string>
#include <system_error>

/// \file parser.h Parser for a busrpc project directory.

namespace google { namespace protobuf {
class Descriptor;
class DescriptorProto;
class EnumDescriptor;
class FieldDescriptor;
class FieldDescriptorProto;
class FileDescriptor;
class FileDescriptorProto;

namespace compiler {
class Importer;
}

}} // namespace google::protobuf

namespace busrpc {

/// Parser error code.
enum class ParserErrc {
    Read_Failed = 1,   ///< Failed to read protobuf file (this code is also used if directory can't be read).
    Protobuf_Error = 2 ///< Error reported by the internally used protobuf parser.
};

/// Return parser error category.
const std::error_category& parser_error_category();

/// Create error code from the \ref ParserErrc value.
std::error_code make_error_code(ParserErrc errc);

/// \note Reads files with \a .proto extension and builds \ref Project from them.
class Parser {
public:
    /// Create parser for \a projectDir.
    /// \note Project directory is the directory which contains \a api/ and \a services/ subdirectories.
    /// \note Paremeter \a protobufRoot allows to specify where to search for built-in \a .proto files provided by
    ///       the protobuf library (for example, 'google/protobuf/descriptor.proto', etc.). On *nix systems parser
    ///       additionally searches for built-in \a .proto files in '/usr/include' and '/usr/local/include' if
    ///       \a protobufRoot is not set or does not contain necessary file.
    explicit Parser(std::filesystem::path projectDir = std::filesystem::current_path(),
                    std::filesystem::path protobufRoot = {}) noexcept:
        projectDir_(std::move(projectDir)),
        protobufRoot_(std::move(protobufRoot))
    { }

    /// Return project directory.
    const std::filesystem::path& projectDir() const noexcept { return projectDir_; }

    /// Return protobuf root directory where to search for built-in \a .proto files.
    const std::filesystem::path& protobufRoot() const noexcept { return protobufRoot_; }

    /// Parse project directory and build \ref Project.
    /// \note Parser does not stop working when error is encountered, which means that returned project may be
    ///       incomplete if errors are found.
    ///  \note Uses default error collector, which assumes the following priorities of the error codes:
    ///        <tt>FilesystemErrc > SpecErrc > DocsErrc > SpecWarn > StyleErrc</tt>
    std::pair<ProjectPtr, ErrorCollector> parse() const;

    /// Parse project directory and build \ref Project.
    /// \note Parser does not stop working when error is encountered, which means that returned project may be
    ///       incomplete if errors are found.
    ProjectPtr parse(ErrorCollector& errorCollector) const;

private:
    GeneralCompositeEntity* visitSubdirectory(GeneralCompositeEntity* parent,
                                              ErrorCollector& ecol,
                                              const std::string& subdirName) const;
    void parseDir(google::protobuf::compiler::Importer& importer,
                  GeneralCompositeEntity* entity,
                  ErrorCollector& ecol) const;
    void parseFile(const google::protobuf::FileDescriptor* fileDesc,
                   const google::protobuf::FileDescriptorProto* fileDescProto,
                   GeneralCompositeEntity* entity,
                   ErrorCollector& ecol) const;

    void addEnum(GeneralCompositeEntity* entity,
                 const google::protobuf::EnumDescriptor* desc,
                 const std::string& filename = {}) const;
    void initEnum(Enum* enumeration, const google::protobuf::EnumDescriptor* desc) const;

    void addStruct(GeneralCompositeEntity* entity,
                   const google::protobuf::Descriptor* desc,
                   const google::protobuf::DescriptorProto* descProto,
                   const std::string& filename = {}) const;
    void initStruct(Struct* structure,
                    const google::protobuf::Descriptor* desc,
                    const google::protobuf::DescriptorProto* descProto) const;

    void addField(Struct* structure,
                  const google::protobuf::FieldDescriptor* desc,
                  const google::protobuf::FieldDescriptorProto* descProto) const;

    std::filesystem::path projectDir_;
    std::filesystem::path protobufRoot_;
};
} // namespace busrpc

namespace std {
template<>
struct is_error_code_enum<busrpc::ParserErrc>: true_type { };
} // namespace std
