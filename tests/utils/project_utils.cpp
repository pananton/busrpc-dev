#include "utils/project_utils.h"

namespace busrpc { namespace test {

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
    return "/* Test enum.\n"
           "Test enum long description.*/\n"
           "enum TestEnum {\n"
           "  // Constant 0.\n"
           "  TEST_ENUM_0 = 0;\n"
           "  // Constant 1.\n"
           "  // Constant 1 long description.\n"
           "  TEST_ENUM_1 = 1;\n"
           "}\n"
           "\n//Dangling comment\n\n";
}

std::string GetTestStruct()
{
    return "// Test struct.\n"
           "// Test struct long description.\n"
           "message TestStruct {\n"
           "  option (hashed_struct) = true;\n"
           "  // Nested test enum.\n"
           "  enum NestedTestEnum {\n"
           "    // Constant 0.\n"
           "    NESTED_TEST_ENUM_0 = 0;\n"
           "  }\n"
           "\n"
           "  /* Nested test struct. */\n"
           "  message NestedTestStruct {\n"
           "    // Field 1.\n"
           "    optional int32 field1 = 1 [(observable) = true, (hashed) = true, (default_value) = \"1001\"];\n"
           "    // Field 2.\n"
           "    // Field 2 long description.\n"
           "    repeated string field2 = 2 [(observable) = false, (hashed) = false];\n"
           "    /* Field 3.\n"
           "    Field 3 long description.*/\n"
           "    map<int32, bytes> field3 = 3 [(observable) = false, (hashed) = false];\n"
           "    oneof TestOneof1 {\n"
           "      // Field 4.\n"
           "      /* Field 4 long description. */\n"
           "      uint32 field4 = 4;\n"
           "      /* Field 5.*/\n"
           "      // Field 5 long description.\n"
           "      sint32 field5 = 5;\n"
           "    }\n"
           "    oneof TestOneof2 {\n"
           "      // Field 6.\n"
           "      int32 field6 = 6 [(observable) = false, (hashed) = false];\n"
           "    }\n"
           "  }\n"
           "\n"
           "  // Field 1.\n"
           "  int32 field1 = 1 [(observable) = true];\n"
           "  /* Field 2.*/\n"
           "  bytes field2 = 2 [(hashed) = true];\n"
           "  // Field 3.\n"
           "  optional float field3 = 3 [(default_value) = \"0.5\"];\n"
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
    return "// Method.\n"
           "// \\pre Precondition.\n"
           "// \\post Postcondition.\n"
           "message MethodDesc {\n"
           "  message Params {\n"
           "    // Param 1.\n"
           "    busrpc.api.TestEnum field1 = 1 [(observable) = true, (hashed) = false, (default_value) = \"1\"];\n"
           "    // Param 2.\n"
           "    busrpc.api.namespace.TestStruct field2 = 2 [(hashed) = true, (default_value) = \"world\"];\n"
           "    // Param 3.\n"
           "    map<int32, busrpc.api.namespace.class.TestStruct.NestedTestStruct> field3 = 3;\n"
           "    // Dangling comment.\n"
           "  }\n"
           "\n// Dangling comment.\n\n"
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
           "    map<int32, busrpc.api.namespace.class.TestStruct.NestedTestStruct> field3 = 3;\n"
           "    // Dangling comment.\n"
           "  }\n"
           "\n// Dangling comment.\n\n"
           "  message Retval {\n"
           "    // Field 1.\n"
           "    int32 result = 1;\n"
           "  }\n"
           "\n// Dangling comment.\n\n"
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
           "/* \\email jdoe@company.com\n"
           "\\url git@company.com:jdoe/repo.git */\n"
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
           "    /* \\accept param1 value1 */\n"
           "    busrpc.api.namespace.static_class.static_method.MethodDesc method2 = 2;\n"
           "  }\n"
           "  message Invokes {\n"
           "    // Method 1.\n"
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
    projectDir.writeFile("1.proto", GetFileHeader("busrpc") + GetTestEnum() + GetTestStruct());

    projectDir.writeFile("api/1.proto", GetFileHeader("busrpc.api") + GetTestEnum() + GetTestStruct());

    projectDir.writeFile("api/namespace/namespace.proto",
                         GetFileHeader("busrpc.api.namespace") + GetNamespaceDescriptor());
    projectDir.writeFile("api/namespace/1.proto", GetFileHeader("busrpc.api.namespace") + GetTestEnum());
    projectDir.writeFile("api/namespace/2.proto", GetFileHeader("busrpc.api.namespace") + GetTestStruct());

    projectDir.writeFile("api/namespace/class/class.proto",
                         GetFileHeader("busrpc.api.namespace.class") + GetClassDescriptor() + GetTestEnum() +
                             GetTestStruct());

    projectDir.writeFile("api/namespace/static_class/class.proto",
                         GetFileHeader("busrpc.api.namespace.static_class") + GetStaticClassDescriptor());
    projectDir.writeFile("api/namespace/static_class/1.proto",
                         GetFileHeader("busrpc.api.namespace.static_class") + GetTestEnum());
    projectDir.writeFile("api/namespace/static_class/2.proto",
                         GetFileHeader("busrpc.api.namespace.static_class") + GetTestStruct());

    projectDir.writeFile("api/namespace/class/method/method.proto",
                         GetFileHeader("busrpc.api.namespace.class.method",
                                       {"api/1.proto", "api/namespace/2.proto", "api/namespace/class/class.proto"}) +
                             GetMethodDescriptor());
    projectDir.writeFile("api/namespace/class/oneway_method/method.proto",
                         GetFileHeader("busrpc.api.namespace.class.oneway_method") + GetOnewayMethodDescriptor());
    projectDir.writeFile("api/namespace/class/oneway_static_method/method.proto",
                         GetFileHeader("busrpc.api.namespace.class.oneway_static_method") +
                             GetOnewayStaticMethodDescriptor());

    projectDir.writeFile("api/namespace/static_class/static_method/method.proto",
                         GetFileHeader("busrpc.api.namespace.static_class.static_method",
                                       {"api/1.proto", "api/namespace/2.proto", "api/namespace/class/class.proto"}) +
                             GetStaticMethodDescriptor());

    projectDir.writeFile("services/1.proto", GetFileHeader("busrpc.services") + GetTestEnum());
    projectDir.writeFile("services/2.proto", GetFileHeader("busrpc.services") + GetTestStruct());

    projectDir.writeFile("services/service/service.proto",
                         GetFileHeader("busrpc.services.service",
                                       {"api/namespace/class/method/method.proto",
                                        "api/namespace/class/oneway_method/method.proto",
                                        "api/namespace/class/oneway_static_method/method.proto",
                                        "api/namespace/static_class/static_method/method.proto"}) +
                             GetServiceDescriptor());
}
}} // namespace busrpc::test
