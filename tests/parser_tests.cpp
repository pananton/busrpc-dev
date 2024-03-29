#include "parser/parser.h"
#include "tests_configure.h"
#include "utils/common.h"
#include "utils/project_utils.h"

#include <gtest/gtest.h>

namespace busrpc { namespace test {

TEST(ParserTest, File_Error_Category_Name_Is_Not_Empty)
{
    EXPECT_TRUE(parser_error_category().name());
    EXPECT_NE(parser_error_category().name()[0], 0);
}

TEST(ParserTest, File_Error_Codes_Have_Non_Empty_Descriptions)
{
    EXPECT_FALSE(parser_error_category().message(static_cast<int>(ParserErrc::Read_Failed)).empty());
    EXPECT_FALSE(parser_error_category().message(static_cast<int>(ParserErrc::Protobuf_Error)).empty());
}

TEST(ParserTest, Unknown_File_Error_Code_Has_Non_Empty_Description)
{
    EXPECT_FALSE(parser_error_category().message(0).empty());
}

TEST(ParserTest, Ctor_Correctly_Initiliazes_Object)
{
    std::filesystem::path projectDir("project_dir");
    std::filesystem::path protobufRoot("protobuf_root");
    Parser parser(projectDir, protobufRoot);

    EXPECT_EQ(parser.projectDir(), projectDir);
    EXPECT_EQ(parser.protobufRoot(), protobufRoot);
}

TEST(ParserTest, Parser_Correctly_Parses_Test_Project)
{
    TmpDir dir;
    CreateTestProject(dir);

    Parser parser(dir.path(), BUSRPC_TESTS_PROTOBUF_ROOT);
    auto [project, ecol] = parser.parse();

    EXPECT_EQ(ecol.errors().size(), 0);
    ASSERT_TRUE(project);

    // most of the checks are done by Project::check, for which dedicated tests exist

    {
        auto errc = project->errc();

        ASSERT_TRUE(errc);
        EXPECT_EQ(errc->constants().size(), 1);
        EXPECT_NE(errc->constants().find("ERRC_UNEXPECTED"), errc->constants().end());
    }

    {
        auto exception = project->exception();

        ASSERT_TRUE(exception);
        EXPECT_EQ(exception->fields().size(), 1);
        EXPECT_NE(exception->fields().find(Exception_Code_Field_Name), exception->fields().end());
    }

    {
        auto callMessage = project->callMessage();

        ASSERT_TRUE(callMessage);
        EXPECT_EQ(callMessage->fields().size(), 2);
        EXPECT_NE(callMessage->fields().find(Call_Message_Object_Id_Field_Name), callMessage->fields().end());
        EXPECT_NE(callMessage->fields().find(Call_Message_Params_Field_Name), callMessage->fields().end());
    }

    {
        auto resultMessage = project->resultMessage();

        ASSERT_TRUE(resultMessage);
        EXPECT_EQ(resultMessage->fields().size(), 2);
        EXPECT_NE(resultMessage->fields().find(Result_Message_Retval_Field_Name), resultMessage->fields().end());
        EXPECT_NE(resultMessage->fields().find(Result_Message_Exception_Field_Name), resultMessage->fields().end());
    }

    {
        auto api = project->api();

        ASSERT_TRUE(api);
        EXPECT_EQ(api->namespaces().size(), 1);
        ASSERT_NE(api->namespaces().find("namespace"), api->namespaces().end());
    }

    {
        auto ns = *(project->api()->namespaces().find("namespace"));

        EXPECT_EQ(ns->classes().size(), 2);
        ASSERT_NE(ns->classes().find("class"), ns->classes().end());
        ASSERT_NE(ns->classes().find("static_class"), ns->classes().end());
    }

    {
        auto cls = (*(*(project->api()->namespaces().find("namespace")))->classes().find("class"));

        EXPECT_EQ(cls->methods().size(), 3);
        ASSERT_NE(cls->methods().find("method"), cls->methods().end());
        ASSERT_NE(cls->methods().find("oneway_method"), cls->methods().end());
        ASSERT_NE(cls->methods().find("oneway_static_method"), cls->methods().end());
    }

    {
        auto cls = (*(*(project->api()->namespaces().find("namespace")))->classes().find("static_class"));

        EXPECT_EQ(cls->methods().size(), 1);
        ASSERT_NE(cls->methods().find("static_method"), cls->methods().end());
    }

    {
        auto cls = (*(*(project->api()->namespaces().find("namespace")))->classes().find("class"));
        auto method = *(cls->methods().find("method"));

        EXPECT_EQ(method->precondition(), "Precondition.");
        EXPECT_EQ(method->postcondition(), "Postcondition.");
        ASSERT_TRUE(method->params());
        ASSERT_TRUE(method->retval());
        ASSERT_NE(method->params()->fields().find("field1"), method->params()->fields().end());
        ASSERT_NE(method->params()->fields().find("field2"), method->params()->fields().end());
        ASSERT_NE(method->params()->fields().find("field3"), method->params()->fields().end());
        ASSERT_NE(method->retval()->fields().find("result"), method->retval()->fields().end());

        auto field = *(method->params()->fields().find("field1"));

        EXPECT_EQ(field->fieldType(), FieldTypeId::Enum);
        EXPECT_EQ(field->fieldTypeName(), "busrpc.api.TestEnum");

        field = *(method->params()->fields().find("field2"));

        EXPECT_EQ(field->fieldType(), FieldTypeId::Message);
        EXPECT_EQ(field->fieldTypeName(), "busrpc.api.namespace.TestStruct");

        field = *(method->params()->fields().find("field3"));

        ASSERT_EQ(field->fieldType(), FieldTypeId::Map);

        auto mapField = static_cast<const MapField*>(field);

        EXPECT_EQ(mapField->keyType(), FieldTypeId::Int32);
        EXPECT_EQ(mapField->valueType(), FieldTypeId::Message);
        EXPECT_EQ(mapField->valueTypeName(), "busrpc.TestStruct.NestedTestStruct");
    }

    {
        auto implementation = project->implementation();

        ASSERT_TRUE(implementation);
        EXPECT_EQ(implementation->services().size(), 1);
        ASSERT_NE(implementation->services().find("service"), implementation->services().end());
    }

    {
        auto cls = (*(*(project->api()->namespaces().find("namespace")))->classes().find("class"));
        auto static_cls = (*(*(project->api()->namespaces().find("namespace")))->classes().find("static_class"));
        auto method = *(cls->methods().find("method"));
        auto onewayMethod = *(cls->methods().find("oneway_method"));
        auto onewayStaticMethod = *(cls->methods().find("oneway_static_method"));
        auto staticMethod = *(static_cls->methods().find("static_method"));
        auto service = *(project->implementation()->services().find("service"));

        EXPECT_EQ(service->author(), "John Doe");
        EXPECT_EQ(service->email(), "jdoe@company.com");
        EXPECT_EQ(service->url(), "git@company.com:jdoe/repo.git");

        EXPECT_EQ(service->implementedMethods().size(), 2);
        ASSERT_NE(service->implementedMethods().find(method->dname()), service->implementedMethods().end());
        ASSERT_NE(service->implementedMethods().find(staticMethod->dname()), service->implementedMethods().end());

        EXPECT_EQ(service->invokedMethods().size(), 2);
        ASSERT_NE(service->invokedMethods().find(onewayMethod->dname()), service->invokedMethods().end());
        ASSERT_NE(service->invokedMethods().find(onewayStaticMethod->dname()), service->invokedMethods().end());

        const auto& implMethod = *(service->implementedMethods().find(method->dname()));
        const auto& implStaticMethod = *(service->implementedMethods().find(staticMethod->dname()));

        EXPECT_EQ(implMethod.docs().brief(), " Method 1.");
        EXPECT_EQ(implMethod.docs().description().size(), 1);
        EXPECT_EQ(implMethod.docs().commands().size(), 1);
        ASSERT_NE(implMethod.docs().commands().find(doc_cmd::Accepted_Value), implMethod.docs().commands().end());
        EXPECT_EQ(implMethod.docs().commands().find(doc_cmd::Accepted_Value)->second.size(), 2);
        ASSERT_TRUE(implMethod.acceptedObjectId());
        EXPECT_EQ(*implMethod.acceptedObjectId(), "1");
        EXPECT_EQ(implMethod.acceptedParams().size(), 1);
        ASSERT_NE(implMethod.acceptedParams().find("param1"), implMethod.acceptedParams().end());
        EXPECT_EQ(implMethod.acceptedParams().find("param1")->second, "value1");

        EXPECT_EQ(implStaticMethod.docs().brief(), " Method 2.");
        EXPECT_EQ(implStaticMethod.docs().description().size(), 1);
        EXPECT_EQ(implStaticMethod.docs().commands().size(), 1);
        ASSERT_NE(implStaticMethod.docs().commands().find(doc_cmd::Accepted_Value),
                  implStaticMethod.docs().commands().end());
        EXPECT_EQ(implStaticMethod.docs().commands().find(doc_cmd::Accepted_Value)->second.size(), 1);
        EXPECT_FALSE(implStaticMethod.acceptedObjectId());
        EXPECT_EQ(implStaticMethod.acceptedParams().size(), 1);
        ASSERT_NE(implStaticMethod.acceptedParams().find("param2"), implStaticMethod.acceptedParams().end());
        EXPECT_EQ(implStaticMethod.acceptedParams().find("param2")->second, "value2");

        const auto& invkMethod = *(service->invokedMethods().find(onewayMethod->dname()));
        const auto& invkStaticMethod = *(service->invokedMethods().find(onewayStaticMethod->dname()));

        EXPECT_EQ(invkMethod.docs().brief(), " Method 1.");
        ASSERT_EQ(invkMethod.docs().description().size(), 2);
        EXPECT_EQ(invkMethod.docs().description()[1], " Long description.");
        EXPECT_EQ(invkMethod.docs().commands().size(), 0);

        EXPECT_EQ(invkStaticMethod.docs().brief(), " Method 2.");
        EXPECT_EQ(invkStaticMethod.docs().description().size(), 1);
        EXPECT_EQ(invkStaticMethod.docs().commands().size(), 0);
    }

    auto testEnum = [](const Enum* enumeration) {
        EXPECT_EQ(enumeration->name(), "TestEnum");
        EXPECT_EQ(enumeration->file().filename(),
                  JoinStrings(GetEntityTypeIdStr(enumeration->parent()->type()), "_types.proto"));
        EXPECT_EQ(enumeration->constants().size(), 2);
        ASSERT_NE(enumeration->constants().find("TEST_ENUM_0"), enumeration->constants().end());
        ASSERT_NE(enumeration->constants().find("TEST_ENUM_1"), enumeration->constants().end());
        EXPECT_EQ(enumeration->docs().brief(), " Test enum. ");
        EXPECT_EQ(enumeration->docs().commands().size(), 0);
        ASSERT_EQ(enumeration->docs().description().size(), 2);
        EXPECT_EQ(enumeration->docs().description()[1], "Test enum long description.");

        auto constant = *(enumeration->constants().find("TEST_ENUM_0"));

        EXPECT_EQ(constant->name(), "TEST_ENUM_0");
        EXPECT_EQ(constant->value(), 0);
        EXPECT_EQ(constant->docs().brief(), " Constant 0.");
        EXPECT_EQ(constant->docs().commands().size(), 0);
        EXPECT_EQ(constant->docs().description().size(), 1);

        constant = *(enumeration->constants().find("TEST_ENUM_1"));

        EXPECT_EQ(constant->name(), "TEST_ENUM_1");
        EXPECT_EQ(constant->value(), 13);
        EXPECT_EQ(constant->docs().brief(), "Constant 1.");
        EXPECT_EQ(constant->docs().commands().size(), 0);
        ASSERT_EQ(constant->docs().description().size(), 2);
        EXPECT_EQ(constant->docs().description()[1], " Constant 1 long description.");
    };

    auto testNestedEnum = [](const Enum* enumeration) {
        EXPECT_EQ(enumeration->name(), "NestedTestEnum");
        EXPECT_EQ(enumeration->file().filename(), static_cast<const Struct*>(enumeration->parent())->file().filename());
        EXPECT_EQ(enumeration->constants().size(), 1);
        ASSERT_NE(enumeration->constants().find("NESTED_TEST_ENUM_0"), enumeration->constants().end());
        EXPECT_EQ(enumeration->docs().brief(), "Nested test enum.");
        EXPECT_EQ(enumeration->docs().commands().size(), 0);
        EXPECT_EQ(enumeration->docs().description().size(), 1);

        auto constant = *(enumeration->constants().find("NESTED_TEST_ENUM_0"));

        EXPECT_EQ(constant->name(), "NESTED_TEST_ENUM_0");
        EXPECT_EQ(constant->value(), 0);
        EXPECT_EQ(constant->docs().brief(), " Constant 0.");
        EXPECT_EQ(constant->docs().commands().size(), 0);
        EXPECT_EQ(constant->docs().description().size(), 1);
    };

    auto testNestedStruct = [](const Struct* structure) {
        EXPECT_EQ(structure->name(), "NestedTestStruct");
        EXPECT_EQ(structure->file().filename(), static_cast<const Struct*>(structure->parent())->file().filename());
        EXPECT_EQ(structure->flags(), StructFlags::None);
        EXPECT_EQ(structure->docs().brief(), " Nested test struct. ");
        EXPECT_EQ(structure->docs().description().size(), 1);
        EXPECT_EQ(structure->docs().commands().size(), 0);

        EXPECT_EQ(structure->fields().size(), 6);
        ASSERT_NE(structure->fields().find("field1"), structure->fields().end());
        ASSERT_NE(structure->fields().find("field2"), structure->fields().end());
        ASSERT_NE(structure->fields().find("field3"), structure->fields().end());
        ASSERT_NE(structure->fields().find("field4"), structure->fields().end());
        ASSERT_NE(structure->fields().find("field5"), structure->fields().end());
        ASSERT_NE(structure->fields().find("field6"), structure->fields().end());

        EXPECT_EQ(structure->enums().size(), 0);
        EXPECT_EQ(structure->structs().size(), 0);

        auto field = *(structure->fields().find("field1"));

        EXPECT_EQ(field->name(), "field1");
        EXPECT_EQ(field->number(), 1);
        EXPECT_EQ(field->fieldType(), FieldTypeId::Int32);
        EXPECT_EQ(field->flags(), FieldFlags::Optional | FieldFlags::Observable | FieldFlags::Hashed);
        EXPECT_TRUE(field->oneofName().empty());
        EXPECT_EQ(field->defaultValue(), "1001");
        EXPECT_EQ(field->docs().brief(), "Field 1.");
        EXPECT_EQ(field->docs().description().size(), 1);
        EXPECT_EQ(field->docs().commands().size(), 0);

        field = *(structure->fields().find("field2"));

        EXPECT_EQ(field->name(), "field2");
        EXPECT_EQ(field->number(), 2);
        EXPECT_EQ(field->fieldType(), FieldTypeId::String);
        EXPECT_EQ(field->flags(), FieldFlags::Repeated);
        EXPECT_TRUE(field->oneofName().empty());
        EXPECT_TRUE(field->defaultValue().empty());
        EXPECT_EQ(field->docs().brief(), "Field 2.");
        ASSERT_EQ(field->docs().description().size(), 2);
        EXPECT_EQ(field->docs().description()[1], "Field 2 long description.");
        EXPECT_EQ(field->docs().commands().size(), 0);

        field = *(structure->fields().find("field3"));

        EXPECT_EQ(field->name(), "field3");
        EXPECT_EQ(field->number(), 3);
        ASSERT_EQ(field->fieldType(), FieldTypeId::Map);
        EXPECT_EQ(static_cast<const MapField*>(field)->keyType(), FieldTypeId::Int32);
        EXPECT_EQ(static_cast<const MapField*>(field)->valueType(), FieldTypeId::Bytes);
        EXPECT_EQ(field->flags(), FieldFlags::None);
        EXPECT_TRUE(field->oneofName().empty());
        EXPECT_TRUE(field->defaultValue().empty());
        EXPECT_EQ(field->docs().brief(), "Field 3.");
        ASSERT_EQ(field->docs().description().size(), 2);
        EXPECT_EQ(field->docs().description()[1], "Field 3 long description.");
        EXPECT_EQ(field->docs().commands().size(), 0);

        field = *(structure->fields().find("field4"));

        EXPECT_EQ(field->name(), "field4");
        EXPECT_EQ(field->number(), 4);
        EXPECT_EQ(field->fieldType(), FieldTypeId::Uint32);
        EXPECT_EQ(field->flags(), FieldFlags::None);
        EXPECT_EQ(field->oneofName(), "TestOneof1");
        EXPECT_TRUE(field->defaultValue().empty());
        EXPECT_EQ(field->docs().brief(), "Field 4.");
        EXPECT_EQ(field->docs().description().size(), 1);
        EXPECT_EQ(field->docs().commands().size(), 0);

        field = *(structure->fields().find("field5"));

        EXPECT_EQ(field->name(), "field5");
        EXPECT_EQ(field->number(), 5);
        EXPECT_EQ(field->fieldType(), FieldTypeId::Sint32);
        EXPECT_EQ(field->flags(), FieldFlags::None);
        EXPECT_EQ(field->oneofName(), "TestOneof1");
        EXPECT_EQ(field->defaultValue(), "100");
        EXPECT_EQ(field->docs().brief(), "Field 5.");
        EXPECT_EQ(field->docs().description().size(), 1);
        EXPECT_EQ(field->docs().commands().size(), 0);

        field = *(structure->fields().find("field6"));

        EXPECT_EQ(field->name(), "field6");
        EXPECT_EQ(field->number(), 6);
        EXPECT_EQ(field->fieldType(), FieldTypeId::Float);
        EXPECT_EQ(field->flags(), FieldFlags::None);
        EXPECT_EQ(field->oneofName(), "TestOneof2");
        EXPECT_TRUE(field->defaultValue().empty());
        EXPECT_EQ(field->docs().brief(), "Field 6.");
        EXPECT_EQ(field->docs().description().size(), 1);
        EXPECT_EQ(field->docs().commands().size(), 0);
    };

    auto testStruct = [&](const Struct* structure) {
        EXPECT_EQ(structure->name(), "TestStruct");
        EXPECT_EQ(structure->file().filename(),
                  JoinStrings(GetEntityTypeIdStr(structure->parent()->type()), "_types.proto"));
        EXPECT_EQ(structure->flags(), StructFlags::Hashed);
        EXPECT_EQ(structure->docs().brief(), " Test struct.");
        ASSERT_EQ(structure->docs().description().size(), 2);
        EXPECT_EQ(structure->docs().description()[1], " Test struct long description.");

        EXPECT_EQ(structure->fields().size(), 3);
        ASSERT_NE(structure->fields().find("field1"), structure->fields().end());
        ASSERT_NE(structure->fields().find("field2"), structure->fields().end());
        ASSERT_NE(structure->fields().find("field3"), structure->fields().end());

        EXPECT_EQ(structure->enums().size(), 1);
        ASSERT_NE(structure->enums().find("NestedTestEnum"), structure->enums().end());

        EXPECT_EQ(structure->structs().size(), 1);
        ASSERT_NE(structure->structs().find("NestedTestStruct"), structure->structs().end());

        auto field = *(structure->fields().find("field1"));

        EXPECT_EQ(field->name(), "field1");
        EXPECT_EQ(field->number(), 1);
        EXPECT_EQ(field->fieldType(), FieldTypeId::Int32);
        EXPECT_EQ(field->flags(), FieldFlags::Observable);
        EXPECT_TRUE(field->oneofName().empty());
        EXPECT_TRUE(field->defaultValue().empty());
        EXPECT_EQ(field->docs().brief(), " Field 1.");
        EXPECT_EQ(field->docs().description().size(), 1);
        EXPECT_EQ(field->docs().commands().size(), 0);

        field = *(structure->fields().find("field2"));

        EXPECT_EQ(field->name(), "field2");
        EXPECT_EQ(field->number(), 2);
        EXPECT_EQ(field->fieldType(), FieldTypeId::Bytes);
        EXPECT_EQ(field->flags(), FieldFlags::Hashed);
        EXPECT_TRUE(field->oneofName().empty());
        EXPECT_TRUE(field->defaultValue().empty());
        EXPECT_EQ(field->docs().brief(), " Field 2.");
        EXPECT_EQ(field->docs().description().size(), 1);
        EXPECT_EQ(field->docs().commands().size(), 0);

        field = *(structure->fields().find("field3"));

        EXPECT_EQ(field->name(), "field3");
        EXPECT_EQ(field->number(), 3);
        EXPECT_EQ(field->fieldType(), FieldTypeId::String);
        EXPECT_EQ(field->flags(), FieldFlags::Optional | FieldFlags::Observable);
        EXPECT_TRUE(field->oneofName().empty());
        EXPECT_EQ(field->defaultValue(), "test");
        EXPECT_EQ(field->docs().brief(), " Field 3.");
        EXPECT_EQ(field->docs().description().size(), 1);
        EXPECT_EQ(field->docs().commands().size(), 0);

        testNestedEnum(*(structure->enums().find("NestedTestEnum")));
        testNestedStruct(*(structure->structs().find("NestedTestStruct")));
    };

    using CheckEntityFunc = std::function<void(const GeneralCompositeEntity*)>;

    CheckEntityFunc checkEntity = [&](const GeneralCompositeEntity* entity) {
        EXPECT_EQ(entity->enums().size(), entity->type() == EntityTypeId::Project ? 2 : 1);
        ASSERT_NE(entity->enums().find("TestEnum"), entity->enums().end());
        ASSERT_NE(entity->structs().find("TestStruct"), entity->structs().end());

        testEnum(*(entity->enums().find("TestEnum")));
        testStruct(*(entity->structs().find("TestStruct")));

        for (const auto& nested: entity->nested()) {
            switch (nested->type()) {
            case EntityTypeId::Api:
            case EntityTypeId::Namespace:
            case EntityTypeId::Class:
            case EntityTypeId::Method:
            case EntityTypeId::Implementation:
            case EntityTypeId::Service: checkEntity(static_cast<const GeneralCompositeEntity*>(nested));
            default: break; ;
            }
        }
    };

    checkEntity(project.get());
}

TEST(ParserTest, Invalid_Project_Dir_Parser_Error_If_Project_Directory_Does_Not_Contain_Builtin_File)
{
    TmpDir tmp;
    Parser parser(tmp.path(), BUSRPC_TESTS_PROTOBUF_ROOT);

    EXPECT_TRUE(parser.parse().second.find(ParserErrc::Invalid_Project_Dir));
}

TEST(ParserTest, Unexpected_Nested_Entity_Spec_Warn_If_Unknown_Directory_Is_Found_In_Project_Directory)
{
    TmpDir tmp;
    CreateMinimalProject(tmp);
    tmp.createDir("unknown_dir");
    Parser parser(tmp.path(), BUSRPC_TESTS_PROTOBUF_ROOT);

    EXPECT_TRUE(parser.parse().second.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST(ParserTest, Unexpected_Nested_Entity_Spec_Warn_If_Directory_Is_Found_In_Method_Directory)
{
    TmpDir tmp;
    CreateTestProject(tmp);
    tmp.createDir("api/namespace/class/method/some_dir");
    Parser parser(tmp.path(), BUSRPC_TESTS_PROTOBUF_ROOT);

    EXPECT_TRUE(parser.parse().second.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST(ParserTest, Unexpected_Nested_Entity_Spec_Warn_If_Directory_Is_Found_In_Service_Directory)
{
    TmpDir tmp;
    CreateTestProject(tmp);
    tmp.createDir(JoinStrings(Implementation_Entity_Name, "service/some_dir"));
    Parser parser(tmp.path(), BUSRPC_TESTS_PROTOBUF_ROOT);

    EXPECT_TRUE(parser.parse().second.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST(ParserTest, Unexpected_Directories_Are_Ignored_By_Parser)
{
    TmpDir tmp;
    CreateMinimalProject(tmp);
    tmp.writeFile("unknown_dir/file.proto", "invalid protobuf file");
    tmp.writeFile("CallMessage/file.proto", "invalid protobuf file");
    Parser parser(tmp.path(), BUSRPC_TESTS_PROTOBUF_ROOT);

    ErrorCollector ecol = parser.parse().second;

    EXPECT_FALSE(ecol.find(ParserErrc::Protobuf_Error));
}

TEST(ParserTest, Files_With_Extension_Other_Than_proto_Are_Ignored_By_Parser)
{
    TmpDir tmp;
    CreateMinimalProject(tmp);
    tmp.writeFile("file.proto1", "invalid protobuf file");
    Parser parser(tmp.path(), BUSRPC_TESTS_PROTOBUF_ROOT);

    EXPECT_FALSE(parser.parse().second);
}

TEST(ParserTest, Unexpected_Package_Spec_Error_If_File_Content_Is_Not_Placed_Into_Expected_Package)
{
    std::string content = "syntax = \"proto3\";\n"
                          "package busrpc.aaa;\n"
                          "message MyStruct {}\n";
    TmpDir tmp;
    CreateMinimalProject(tmp);
    tmp.writeFile("file.proto", content);
    Parser parser(tmp.path(), BUSRPC_TESTS_PROTOBUF_ROOT);

    EXPECT_TRUE(parser.parse().second.find(SpecErrc::Unexpected_Package));
}

TEST(ParserTest, Multiple_Definitions_Spec_Error_If_Next_Level_Entity_Conflicts_With_One_Of_The_Current_Level_Entities)
{
    std::string content = "syntax = \"proto3\";"
                          "package busrpc;\n"
                          "enum api {\n"
                          "  CONSTANT_0 = 0;\n"
                          "}\n";
    TmpDir tmp;
    CreateMinimalProject(tmp);
    tmp.writeFile("file.proto", content);
    tmp.createDir("api");
    Parser parser(tmp.path(), BUSRPC_TESTS_PROTOBUF_ROOT);

    auto [projectPtr, ecol] = parser.parse();

    EXPECT_TRUE(ecol.find(SpecErrc::Multiple_Definitions));
    EXPECT_FALSE(projectPtr->api());
}

TEST(ParserTest, Invalid_Entity_Spec_Error_If_Entity_Has_Invalid_Name)
{
    TmpDir tmp;
    CreateMinimalProject(tmp);
    tmp.createDir("api/namespace.proto");
    Parser parser(tmp.path(), BUSRPC_TESTS_PROTOBUF_ROOT);

    EXPECT_TRUE(parser.parse().second.find(SpecErrc::Invalid_Entity));
}

TEST(ParserTest, Protobuf_Error_Parser_Error_If_File_Has_Invalid_Protobuf_Syntax)
{
    TmpDir tmp;
    CreateMinimalProject(tmp);
    tmp.writeFile("invalid.proto", "syntax =");
    Parser parser(tmp.path(), BUSRPC_TESTS_PROTOBUF_ROOT);

    EXPECT_TRUE(parser.parse().second.find(ParserErrc::Protobuf_Error));
}

TEST(ParserTest, Default_Severity_Of_Errors_Is_ParserErrc_SpecErrc_SpecWarn_DocWarn_StyleWarn)
{
    std::string namespaceDesc = "syntax = \"proto3\";\n"
                                "package busrpc.api.Namespace;\n"
                                "//Namespace.\n"
                                "message NamespaceDesc {}";
    std::string undocumentedStruct = "syntax = \"proto3\";\n"
                                     "package busrpc;\n"
                                     "message MyStruct {}";

    {
        TmpDir tmp;
        CreateMinimalProject(tmp);
        Parser parser(tmp.path(), BUSRPC_TESTS_PROTOBUF_ROOT);

        tmp.writeFile("invalid.proto", "syntax =");      // invalid syntax, parser error
        tmp.createDir("api/Namespace");                  // non-conformat name (style warn), no descriptor (spec error)
        tmp.createDir("unknown_dir");                    // unexpected nested type (spec warn)
        tmp.writeFile("file.proto", undocumentedStruct); // undocumented entity (doc warn)

        ErrorCollector ecol = parser.parse().second;

        EXPECT_EQ(ecol.majorError()->code.category(), parser_error_category());
    }

    {
        TmpDir tmp;
        CreateMinimalProject(tmp);
        Parser parser(tmp.path(), BUSRPC_TESTS_PROTOBUF_ROOT);

        tmp.createDir("api/Namespace");                  // non-conformat name (style warn), no descriptor (spec error)
        tmp.createDir("unknown_dir");                    // unexpected nested type (spec warn)
        tmp.writeFile("file.proto", undocumentedStruct); // undocumented entity (doc warn)

        ErrorCollector ecol = parser.parse().second;

        EXPECT_EQ(ecol.majorError()->code.category(), spec_error_category());
    }

    {

        TmpDir tmp;
        CreateMinimalProject(tmp);
        Parser parser(tmp.path(), BUSRPC_TESTS_PROTOBUF_ROOT);

        tmp.writeFile("api/Namespace/namespace.proto", namespaceDesc); // non-conformat name (style warn)
        tmp.createDir("unknown_dir");                                  // unexpected nested type (spec warn)
        tmp.writeFile("file.proto", undocumentedStruct);               // undocumented entity (doc warn)

        ErrorCollector ecol = parser.parse().second;

        EXPECT_EQ(ecol.majorError()->code.category(), spec_warn_category());
    }

    {

        TmpDir tmp;
        CreateMinimalProject(tmp);
        Parser parser(tmp.path(), BUSRPC_TESTS_PROTOBUF_ROOT);

        tmp.writeFile("api/Namespace/namespace.proto", namespaceDesc); // non-conformat name (style warn)
        tmp.writeFile("file.proto", undocumentedStruct);               // undocumented entity (doc warn)

        ErrorCollector ecol = parser.parse().second;

        EXPECT_EQ(ecol.majorError()->code.category(), doc_warn_category());
    }

    {

        TmpDir tmp;
        CreateMinimalProject(tmp);
        Parser parser(tmp.path(), BUSRPC_TESTS_PROTOBUF_ROOT);

        tmp.writeFile("api/Namespace/namespace.proto", namespaceDesc); // non-conformat name (style warn)
        ErrorCollector ecol = parser.parse().second;

        EXPECT_EQ(ecol.majorError()->code.category(), style_warn_category());
    }
}
}} // namespace busrpc::test
