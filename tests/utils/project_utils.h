#pragma once

#include "entities/project.h"
#include "utils/file_utils.h"

#include <memory>
#include <string>
#include <vector>

namespace busrpc { namespace test {

template<typename TEntity>
Struct* AddStructsAndEnums(TEntity* entity)
{
    auto parent = entity->parent();

    auto topStruct = entity->addStruct("Struct", "file.proto", StructFlags::None, EntityDocs("Struct."));
    topStruct->addScalarField("field1",
                              1,
                              FieldTypeId::Int32,
                              FieldFlags::Observable | FieldFlags::Hashed,
                              "",
                              "1001",
                              EntityDocs("Field 1."));
    topStruct->addStructField("field2", 2, "google.protobuf.Any", FieldFlags::None, "", EntityDocs("Field 2"));

    if (parent) {
        topStruct->addMapField("field3",
                               3,
                               FieldTypeId::String,
                               FieldTypeId::Message,
                               parent->dname() + ".Struct",
                               EntityDocs("Field 3."));
        topStruct->addStructField("field4",
                                  4,
                                  parent->dname() + ".Struct.NestedStruct",
                                  FieldFlags::Optional | FieldFlags::Observable | FieldFlags::Hashed,
                                  "",
                                  EntityDocs("Field 4."));
    }

    entity->addEnum("Enum", "file.proto", EntityDocs("Enum."))->addConstant("CONSTANT_0", 0, EntityDocs("Constant."));

    auto nestedStruct = topStruct->addStruct("NestedStruct", StructFlags::Hashed, EntityDocs("Nested struct."));
    nestedStruct->addScalarField("field1",
                                 1,
                                 FieldTypeId::Bytes,
                                 FieldFlags::Optional | FieldFlags::Observable | FieldFlags::Hashed,
                                 "",
                                 "",
                                 EntityDocs("Field 1."));

    if (parent) {
        nestedStruct->addEnumField("field2",
                                   2,
                                   parent->dname() + ".Struct.NestedEnum",
                                   FieldFlags::Optional | FieldFlags::Observable | FieldFlags::Hashed,
                                   "",
                                   EntityDocs("Field 2."));
    }

    auto nestedEnum = topStruct->addEnum("NestedEnum", EntityDocs("Nested enum."));
    nestedEnum->addConstant("CONSTANT_0", 0, EntityDocs("Constant 0."));
    nestedEnum->addConstant("CONSTANT_1", 1001, EntityDocs("Constant 1."));

    return topStruct;
}

Enum* AddErrc(Project* project);
Struct* AddException(Project* project);
Struct* AddCallMessage(Project* project);
Struct* AddResultMessage(Project* project);
Namespace* AddNamespace(Api* api);
Struct* AddNamespaceDesc(Namespace* ns);
Class* AddClass(Namespace* ns, bool isStatic = false);
Struct* AddClassDesc(Class* cls, bool isStatic = false);
Method* AddMethod(Class* cls, bool isStatic = false, bool hasParams = true, bool hasRetval = true);
Struct* AddMethodDesc(Method* method, bool isStatic = false, bool hasParams = true, bool hasRetval = true);
Service* AddService(Implementation* implementation, bool hasConfig = true, bool hasImplements = true, bool hasInvokes = true);
Struct* AddServiceDesc(Service* service, bool hasConfig = true, bool hasImplements = true, bool hasInvokes = true);
Api* AddApi(Project* project);
Implementation* AddImplementation(Project* project);
void InitMinimalProject(Project* project);

std::string GetFileHeader(const std::string& packageName,
                          const std::vector<std::string>& imports = {},
                          bool doNotImportMain = false);
std::string GetTestEnum();
std::string GetTestStruct();
std::string GetMainFile();
std::string GetNamespaceDescriptor();
std::string GetClassDescriptor();
std::string GetStaticClassDescriptor();
std::string GetMethodDescriptor();
std::string GetOnewayMethodDescriptor();
std::string GetStaticMethodDescriptor();
std::string GetOnewayStaticMethodDescriptor();
std::string GetServiceDescriptor();

void CreateMinimalProject(TmpDir& projectDir);
void CreateTestProject(TmpDir& projectDir);
}} // namespace busrpc::test