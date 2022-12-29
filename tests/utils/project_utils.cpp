#include "utils/project_utils.h"

namespace busrpc { namespace test {

Enum* AddErrc(Project* project)
{
    auto errc = project->addEnum(Errc_Enum_Name, Busrpc_Builtin_File, EntityDocs("Exception error code."));
    errc->addConstant("ERRC_UNEXPECTED", 0, EntityDocs("Unexpected error."));
    errc->addConstant("ERRC_INTERNAL", 10, EntityDocs("Internal error."));
    return errc;
}

Struct* AddException(Project* project)
{
    auto exception = project->addStruct(GetPredefinedStructName(StructTypeId::Exception),
                                        Busrpc_Builtin_File,
                                        StructFlags::None,
                                        EntityDocs("Method exception."));
    exception->addEnumField(
        Exception_Code_Field_Name, 5, project->dname() + ".Errc", FieldFlags::None, "", EntityDocs("Exception code."));
    return exception;
}

Struct* AddCallMessage(Project* project)
{
    auto call = project->addStruct(GetPredefinedStructName(StructTypeId::Call_Message),
                                   Busrpc_Builtin_File,
                                   StructFlags::None,
                                   EntityDocs("Call message."));
    call->addScalarField(Call_Message_Object_Id_Field_Name,
                         5,
                         FieldTypeId::Bytes,
                         FieldFlags::Optional,
                         "",
                         "",
                         EntityDocs("Object identifier."));
    call->addScalarField(Call_Message_Params_Field_Name,
                         6,
                         FieldTypeId::Bytes,
                         FieldFlags::Optional,
                         "",
                         "",
                         EntityDocs("Method parameters."));
    return call;
}

Struct* AddResultMessage(Project* project)
{
    auto result = project->addStruct(GetPredefinedStructName(StructTypeId::Result_Message),
                                     Busrpc_Builtin_File,
                                     StructFlags::None,
                                     EntityDocs("Call message."));
    result->addScalarField(Result_Message_Retval_Field_Name,
                           5,
                           FieldTypeId::Bytes,
                           FieldFlags::None,
                           "Result",
                           "",
                           EntityDocs("Method return value."));
    result->addStructField(Result_Message_Exception_Field_Name,
                           6,
                           project->dname() + ".Exception",
                           FieldFlags::None,
                           "Result",
                           EntityDocs("Method exception."));
    return result;
}

Namespace* AddNamespace(Api* api)
{
    auto ns = api->addNamespace("namespace");
    AddNamespaceDesc(ns);
    AddStructsAndEnums(ns);
    return ns;
}

Struct* AddNamespaceDesc(Namespace* ns)
{
    return ns->addStruct(GetPredefinedStructName(StructTypeId::Namespace_Desc),
                         Namespace_Desc_File,
                         StructFlags::None,
                         EntityDocs("My namespace."));
}

Class* AddClass(Namespace* ns, bool isStatic)
{
    auto cls = ns->addClass("class");
    AddClassDesc(cls, isStatic);
    AddStructsAndEnums(cls);
    return cls;
}

Struct* AddClassDesc(Class* cls, bool isStatic)
{
    auto desc = cls->addStruct(
        GetPredefinedStructName(StructTypeId::Class_Desc), Class_Desc_File, StructFlags::None, EntityDocs("My class."));

    if (!isStatic) {
        auto oid = desc->addStruct(GetPredefinedStructName(StructTypeId::Class_Object_Id), StructFlags::Hashed);

        oid->addScalarField("field1", 1, FieldTypeId::Int32, FieldFlags::None, "", "", EntityDocs("Field1."));
        oid->addScalarField("field2",
                            2,
                            FieldTypeId::String,
                            FieldFlags::Optional | FieldFlags::Observable | FieldFlags::Hashed,
                            "",
                            "",
                            EntityDocs("Field2."));
    }

    return desc;
}

Method* AddMethod(Class* cls, bool isStatic, bool hasParams, bool hasRetval)
{
    auto method = cls->addMethod("method");
    AddMethodDesc(method, isStatic, hasParams, hasRetval);
    AddStructsAndEnums(method);
    return method;
}

Struct* AddMethodDesc(Method* method, bool isStatic, bool hasParams, bool hasRetval)
{
    auto desc = method->addStruct(GetPredefinedStructName(StructTypeId::Method_Desc),
                                  Method_Desc_File,
                                  StructFlags::None,
                                  EntityDocs({"My method."}, {{"pre", {"precondition"}}, {"post", {"postcondition"}}}));

    if (isStatic) {
        desc->addStruct(GetPredefinedStructName(StructTypeId::Method_Static_Marker),
                        StructFlags::None,
                        EntityDocs("Static marker."));
    }

    if (hasParams) {
        auto params = desc->addStruct(GetPredefinedStructName(StructTypeId::Method_Params));
        params->addScalarField("field1",
                               1,
                               FieldTypeId::String,
                               FieldFlags::Optional | FieldFlags::Observable | FieldFlags::Hashed,
                               "",
                               "",
                               EntityDocs("Field 1."));
    }

    if (hasRetval) {
        auto retval = desc->addStruct(GetPredefinedStructName(StructTypeId::Method_Retval));
        retval->addScalarField("field1",
                               1,
                               FieldTypeId::Int32,
                               FieldFlags::Optional | FieldFlags::Observable | FieldFlags::Hashed,
                               "",
                               "",
                               EntityDocs("Field 1."));
    }

    return desc;
}

Service* AddService(Implementation* implementation, bool hasConfig, bool hasImplements, bool hasInvokes)
{
    auto service = implementation->addService("service");
    AddServiceDesc(service, hasConfig, hasImplements, hasInvokes);
    AddStructsAndEnums(service);
    return service;
}

Struct* AddServiceDesc(Service* service, bool hasConfig, bool hasImplements, bool hasInvokes)
{
    EntityDocs serviceDesc({"Service."},
                           {{doc_cmd::Service_Author, {"John Doe"}},
                            {doc_cmd::Service_Email, {"john@company.com"}},
                            {doc_cmd::Service_Url, {"company.com"}}});
    auto desc = service->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc),
                                   Service_Desc_File,
                                   StructFlags::None,
                                   std::move(serviceDesc));

    if (hasConfig) {
        auto config = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Config));
        config->addScalarField("field1",
                               1,
                               FieldTypeId::Int32,
                               FieldFlags::Optional | FieldFlags::Observable | FieldFlags::Hashed,
                               "",
                               "1001",
                               EntityDocs("Field 1."));
        config->addScalarField(
            "field2", 2, FieldTypeId::String, FieldFlags::None, "oneofName", "", EntityDocs("Field 2."));
    }

    if (hasImplements) {
        EntityDocs implementedMethodDesc({"Implemented method."}, {{doc_cmd::Accepted_Value, {"@object_id 1"}}});
        auto implements = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Implements), StructFlags::None);
        implements->addStructField("method1",
                                   1,
                                   std::string(Project_Entity_Name) + "." + Api_Entity_Name +
                                       ".namespace.class.method." + GetPredefinedStructName(StructTypeId::Method_Desc),
                                   FieldFlags::Repeated,
                                   "",
                                   std::move(implementedMethodDesc));
    }

    if (hasInvokes) {
        auto invokes = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Invokes));
        invokes->addStructField("method1",
                                1,
                                std::string(Project_Entity_Name) + "." + Api_Entity_Name + ".namespace.class.method." +
                                    GetPredefinedStructName(StructTypeId::Method_Desc),
                                FieldFlags::None,
                                "oneofName",
                                EntityDocs("Invoked method 1."));
    }

    return desc;
}

Api* AddApi(Project* project)
{
    auto api = project->addApi();
    AddStructsAndEnums(api);
    return api;
}

Implementation* AddImplementation(Project* project)
{
    auto impl = project->addImplementation();
    AddStructsAndEnums(impl);
    return impl;
}

void InitMinimalProject(Project* project)
{
    AddErrc(project);
    AddException(project);
    AddCallMessage(project);
    AddResultMessage(project);
}

std::string GetFileHeader(const std::string& packageName, const std::vector<std::string>& imports, bool doNotImportMain)
{
    std::string header = "syntax = \"proto3\";\n"
                         "package ";
    header.append(packageName);
    header.append(";\n");

    if (!doNotImportMain) {
        header.append("import \"busrpc.proto\";\n");
    }

    for (const auto& file: imports) {
        header.append("import \"");
        header.append(file);
        header.append("\";\n");
    }

    return header;
}

std::string GetTestEnum()
{
    return "/* Test enum. \n"
           "Test enum long description.*/\n"
           "enum TestEnum {\n"
           "  // Constant 0.\n"
           "  TEST_ENUM_0 = 0;\n"
           "  // Dangling comment.\n\n"
           "  //Constant 1.\n"
           "  // Constant 1 long description.\n"
           "  TEST_ENUM_1 = 13;\n"
           "}\n"
           "\n//Dangling comment\n\n";
}

std::string GetTestStruct()
{
    return "// Test struct.\n"
           "// Test struct long description.\n"
           "message TestStruct {\n"
           "  option (hashed_struct) = true;\n"
           "  //Nested test enum.\n"
           "  enum NestedTestEnum {\n"
           "    /* Constant 0.*/\n"
           "    NESTED_TEST_ENUM_0 = 0;\n"
           "  }\n"
           "\n"
           "  /* Nested test struct. */\n"
           "  message NestedTestStruct {\n"
           "    //Field 1.\n"
           "    optional int32 field1 = 1 [(observable) = true, (hashed) = true, (default_value) = \"1001\"];\n"
           "    //Field 2.\n"
           "    //Field 2 long description.\n"
           "    repeated string field2 = 2 [(observable) = false, (hashed) = false];\n"
           "    /*Field 3.\n"
           "    Field 3 long description.*/\n"
           "    map<int32, bytes> field3 = 3 [(observable) = false, (hashed) = false];\n"
           "    oneof TestOneof1 {\n"
           "      /*Field 4.*/\n"
           "      uint32 field4 = 4;\n"
           "      //Field 5.\n"
           "      sint32 field5 = 5 [(default_value) = \"100\"];\n"
           "    }\n"
           "    oneof TestOneof2 {\n"
           "      //Field 6.\n"
           "      float field6 = 6 [(observable) = false, (hashed) = false];\n"
           "    }\n"
           "  }\n"
           "\n"
           "  // Field 1.\n"
           "  int32 field1 = 1 [(observable) = true];\n"
           "\n"
           "  // Dangling comment.\n\n"
           "  /* Field 2.*/\n"
           "  bytes field2 = 2 [(hashed) = true];\n"
           "  // Field 3.\n"
           "  optional string field3 = 3 [(observable) = true, (default_value) = \"test\"];\n"
           "}\n"
           "\n//Dangling comment\n\n";
}

std::string GetMainFile()
{
    return GetFileHeader("busrpc", {"google/protobuf/descriptor.proto"}, true) +
           "// Errc\n"
           "enum Errc {\n"
           "  // ERRC_UNEXPECTED\n"
           "  ERRC_UNEXPECTED = 0;\n"
           "}\n"
           "\n"
           "// Exception\n"
           "message Exception {\n"
           "  // code\n"
           "  Errc code = 1;\n"
           "}\n"
           ""
           "// CallMessage\n"
           "message CallMessage {\n"
           "  // object_id\n"
           "  optional bytes object_id = 1;\n"
           "  // params\n"
           "  optional bytes params = 2;\n"
           "}\n"
           "\n"
           "// ResultMessage\n"
           "message ResultMessage {\n"
           "  oneof Result {\n"
           "    // retval\n"
           "    bytes retval = 1;\n"
           "    // exception\n"
           "    Exception exception = 2;\n"
           "  }\n"
           "}\n"
           "\n"
           "extend google.protobuf.MessageOptions {\n"
           "  optional bool hashed_struct = 10000;\n"
           "}\n"
           "\n"
           "extend google.protobuf.FieldOptions {\n"
           "  optional bool observable = 20001;\n"
           "  optional bool hashed = 20002;\n"
           "  optional string default_value = 20003;\n"
           "}\n";
}

std::string GetNamespaceDescriptor()
{
    return "// Namespace.\n"
           "message NamespaceDesc {}\n";
}

std::string GetClassDescriptor()
{
    return "/* Class.\n"
           "Class long description.*/\n"
           "message ClassDesc {\n"
           "  message ObjectId {\n"
           "    // Field 1.\n"
           "    optional int32 field1 = 1;\n"
           "    // Field 2.\n"
           "    bytes field2 = 2;\n"
           "  }\n"
           "}\n";
}

std::string GetStaticClassDescriptor()
{
    return "/* Static class.*/\n"
           "message ClassDesc {}\n";
}

std::string GetMethodDescriptor()
{
    return "/* Method.\n"
           "   \\pre Precondition.\n"
           "   \\post Postcondition. */\n"
           "message MethodDesc {\n"
           "  message Params {\n"
           "    // Param 1.\n"
           "    busrpc.api.TestEnum field1 = 1 [(observable) = true, (hashed) = false, (default_value) = \"1\"];\n"
           "    // Param 2.\n"
           "    busrpc.api.namespace.TestStruct field2 = 2 [(hashed) = true, (default_value) = \"world\"];\n"
           "    // Param 3.\n"
           "    map<int32, busrpc.TestStruct.NestedTestStruct> field3 = 3;\n"
           "  }\n"
           "  message Retval {\n"
           "    // Field 1.\n"
           "    int32 result = 1;\n"
           "  }\n"
           "}\n";
}

std::string GetOnewayMethodDescriptor()
{
    return "// Oneway method.\n"
           "message MethodDesc {}\n";
}

std::string GetStaticMethodDescriptor()
{
    return "// Method.\n"
           "message MethodDesc {\n"
           "  message Params {\n"
           "    // Param 1.\n"
           "    busrpc.api.TestEnum field1 = 1 [(observable) = true, (hashed) = false, (default_value) = \"1\"];\n"
           "    // Param 2.\n"
           "    busrpc.api.namespace.TestStruct field2 = 2 [(hashed) = true, (default_value) = \"value\"];\n"
           "    // Param 3.\n"
           "    map<int32, busrpc.TestStruct.NestedTestStruct> field3 = 3;\n"
           "  }\n"
           "  message Retval {\n"
           "    // Field 1.\n"
           "    int32 result = 1;\n"
           "  }\n"
           "  message Static {}\n"
           "}\n";
}

std::string GetOnewayStaticMethodDescriptor()
{
    return "// Oneway method.\n"
           "message MethodDesc {\n"
           "  message Static {}\n"
           "}\n";
}

std::string GetServiceDescriptor()
{
    return "// Service.\n"
           "// \\author John Doe\n"
           "// \\email jdoe@company.com\n"
           "// \\url git@company.com:jdoe/repo.git\n"
           "// Service long description.\n"
           "message ServiceDesc {\n"
           "  message Config {\n"
           "    // Param 1.\n"
           "    string param1 = 1 [(default_value) = \"value\"];\n"
           "  }\n"
           "  message Implements {\n"
           "    // Method 1.\n"
           "    // \\accept @object_id 1\n"
           "    // \\accept param1 value1\n"
           "    busrpc.api.namespace.class.method.MethodDesc method1 = 1;\n"
           "    // Method 2.\n"
           "    // \\accept param2 value2\n"
           "    busrpc.api.namespace.static_class.static_method.MethodDesc method2 = 2;\n"
           "  }\n"
           "  message Invokes {\n"
           "    // Method 1.\n"
           "    // Long description.\n"
           "    busrpc.api.namespace.class.oneway_method.MethodDesc method1 = 1;\n"
           "    // Method 2.\n"
           "    busrpc.api.namespace.class.oneway_static_method.MethodDesc method2 = 2;\n"
           "  }\n"
           "}\n";
}

void CreateMinimalProject(TmpDir& projectDir)
{
    projectDir.writeFile("busrpc.proto", GetMainFile());
}

void CreateTestProject(TmpDir& projectDir)
{
    CreateMinimalProject(projectDir);
    projectDir.writeFile("project_types.proto", GetFileHeader("busrpc") + GetTestEnum() + GetTestStruct());

    projectDir.writeFile("api/api_types.proto", GetFileHeader("busrpc.api") + GetTestEnum() + GetTestStruct());

    projectDir.writeFile("api/namespace/namespace.proto",
                         GetFileHeader("busrpc.api.namespace") + GetNamespaceDescriptor());
    projectDir.writeFile("api/namespace/namespace_types.proto",
                         GetFileHeader("busrpc.api.namespace") + GetTestEnum() + GetTestStruct());

    projectDir.writeFile("api/namespace/class/class.proto",
                         GetFileHeader("busrpc.api.namespace.class") + GetClassDescriptor());
    projectDir.writeFile("api/namespace/class/class_types.proto",
                         GetFileHeader("busrpc.api.namespace.class") + GetTestEnum() + GetTestStruct());

    projectDir.writeFile("api/namespace/static_class/class.proto",
                         GetFileHeader("busrpc.api.namespace.static_class") + GetStaticClassDescriptor());
    projectDir.writeFile("api/namespace/static_class/class_types.proto",
                         GetFileHeader("busrpc.api.namespace.static_class") + GetTestEnum() + GetTestStruct());

    projectDir.writeFile(
        "api/namespace/class/method/method.proto",
        GetFileHeader("busrpc.api.namespace.class.method",
                      {"project_types.proto", "api/api_types.proto", "api/namespace/namespace_types.proto"}) +
            GetMethodDescriptor());
    projectDir.writeFile("api/namespace/class/method/method_types.proto",
                         GetFileHeader("busrpc.api.namespace.class.method") + GetTestEnum() + GetTestStruct());
    projectDir.writeFile("api/namespace/class/oneway_method/method.proto",
                         GetFileHeader("busrpc.api.namespace.class.oneway_method") + GetOnewayMethodDescriptor());
    projectDir.writeFile("api/namespace/class/oneway_method/method_types.proto",
                         GetFileHeader("busrpc.api.namespace.class.oneway_method") + GetTestEnum() + GetTestStruct());
    projectDir.writeFile("api/namespace/class/oneway_static_method/method.proto",
                         GetFileHeader("busrpc.api.namespace.class.oneway_static_method") +
                             GetOnewayStaticMethodDescriptor());
    projectDir.writeFile("api/namespace/class/oneway_static_method/method_types.proto",
                         GetFileHeader("busrpc.api.namespace.class.oneway_static_method") + GetTestEnum() +
                             GetTestStruct());

    projectDir.writeFile(
        "api/namespace/static_class/static_method/method.proto",
        GetFileHeader("busrpc.api.namespace.static_class.static_method",
                      {"project_types.proto", "api/api_types.proto", "api/namespace/namespace_types.proto"}) +
            GetStaticMethodDescriptor());
    projectDir.writeFile("api/namespace/static_class/static_method/method_types.proto",
                         GetFileHeader("busrpc.api.namespace.static_class.static_method") + GetTestEnum() +
                             GetTestStruct());

    projectDir.writeFile("implementation/implementation_types.proto",
                         GetFileHeader("busrpc.implementation") + GetTestEnum() + GetTestStruct());

    projectDir.writeFile("implementation/service/service.proto",
                         GetFileHeader("busrpc.implementation.service",
                                       {"api/namespace/class/method/method.proto",
                                        "api/namespace/class/oneway_method/method.proto",
                                        "api/namespace/class/oneway_static_method/method.proto",
                                        "api/namespace/static_class/static_method/method.proto"}) +
                             GetServiceDescriptor());
    projectDir.writeFile("implementation/service/service_types.proto",
                         GetFileHeader("busrpc.implementation.service") + GetTestEnum() + GetTestStruct());
}
}} // namespace busrpc::test
