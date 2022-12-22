#include "entities/project.h"

#include <gtest/gtest.h>

namespace busrpc { namespace test {

template<typename TEntity>
Struct* AddStructsAndEnums(TEntity* entity)
{
    auto parent = entity->parent() && entity->parent()->type() != EntityTypeId::Project ? entity->parent() : nullptr;

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

    entity->addEnum("Enum", "file.proto", EntityDocs("Enum."));

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
    nestedEnum->addConstant("CONSTANT_1", 1001, EntityDocs("Constant 1."));

    return topStruct;
}

Enum* AddErrc(Api* api)
{
    auto errc = api->addEnum(Errc_Enum_Name, Busrpc_Builtin_File, EntityDocs("Exception error code."));
    errc->addConstant("ERRC_UNEXPECTED", 0, EntityDocs("Unexpected error."));
    errc->addConstant("ERRC_INTERNAL", 10, EntityDocs("Internal error."));
    return errc;
}

Struct* AddException(Api* api)
{
    auto exception = api->addStruct(GetPredefinedStructName(StructTypeId::Method_Exception),
                                    Busrpc_Builtin_File,
                                    StructFlags::None,
                                    EntityDocs("Method exception."));
    exception->addEnumField(
        Exception_Code_Field_Name, 5, api->dname() + ".Errc", FieldFlags::None, "", EntityDocs("Exception code."));
    return exception;
}

Struct* AddCallMessage(Api* api)
{
    auto call = api->addStruct(GetPredefinedStructName(StructTypeId::Call_Message),
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

Struct* AddResultMessage(Api* api)
{
    auto result = api->addStruct(GetPredefinedStructName(StructTypeId::Result_Message),
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
                           api->dname() + ".Exception",
                           FieldFlags::None,
                           "Result",
                           EntityDocs("Method exception."));
    return result;
}

Struct* AddNamespaceDesc(Namespace* ns)
{
    return ns->addStruct(GetPredefinedStructName(StructTypeId::Namespace_Desc),
                         Namespace_Desc_File,
                         StructFlags::None,
                         EntityDocs("My namespace."));
}

Namespace* AddNamespace(Api* api)
{
    auto ns = api->addNamespace("namespace");
    AddNamespaceDesc(ns);
    AddStructsAndEnums(ns);
    return ns;
}

Struct* AddClassDesc(Class* cls, bool isStatic = false)
{
    auto desc = cls->addStruct(
        GetPredefinedStructName(StructTypeId::Class_Desc), Class_Desc_File, StructFlags::None, EntityDocs("My class."));

    if (!isStatic) {
        auto oid = desc->addStruct(GetPredefinedStructName(StructTypeId::Object_Id), StructFlags::Hashed);

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

Class* AddClass(Namespace* ns, bool isStatic = false)
{
    auto cls = ns->addClass("class");
    AddClassDesc(cls, isStatic);
    AddStructsAndEnums(cls);
    return cls;
}

Struct* AddMethodDesc(Method* method, bool isStatic = false)
{
    auto desc = method->addStruct(GetPredefinedStructName(StructTypeId::Method_Desc),
                                  Method_Desc_File,
                                  StructFlags::None,
                                  EntityDocs("My method."));

    if (isStatic) {
        desc->addStruct(
            GetPredefinedStructName(StructTypeId::Static_Marker), StructFlags::None, EntityDocs("Static marker."));
    }

    auto params = desc->addStruct(GetPredefinedStructName(StructTypeId::Method_Params));
    params->addScalarField("field1",
                           1,
                           FieldTypeId::String,
                           FieldFlags::Optional | FieldFlags::Observable | FieldFlags::Hashed,
                           "",
                           "",
                           EntityDocs("Field 1."));

    auto retval = desc->addStruct(GetPredefinedStructName(StructTypeId::Method_Retval));
    retval->addScalarField("field1",
                           1,
                           FieldTypeId::Int32,
                           FieldFlags::Optional | FieldFlags::Observable | FieldFlags::Hashed,
                           "",
                           "",
                           EntityDocs("Field 1."));

    return desc;
}

Method* AddMethod(Class* cls, bool isStatic = false)
{
    auto method = cls->addMethod("method");
    AddMethodDesc(method, isStatic);
    AddStructsAndEnums(method);
    return method;
}

Struct* AddServiceDesc(Service* service)
{
    EntityDocs serviceDesc({"Service."},
                           {{doc_cmd::Service_Author, {"John Doe"}},
                            {doc_cmd::Service_Email, {"john@company.com"}},
                            {doc_cmd::Service_Url, {"company.com"}}});
    EntityDocs implementedMethodDesc({"Implemented method."}, {{doc_cmd::Accepted_Value, {"@object_id 1"}}});

    auto desc = service->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc),
                                   Service_Desc_File,
                                   StructFlags::None,
                                   std::move(serviceDesc));

    auto config = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Config));
    config->addScalarField("field1",
                           1,
                           FieldTypeId::Int32,
                           FieldFlags::Optional | FieldFlags::Observable | FieldFlags::Hashed,
                           "",
                           "1001",
                           EntityDocs("Field 1."));
    config->addScalarField("field2", 2, FieldTypeId::String, FieldFlags::None, "oneofName", "", EntityDocs("Field 2."));

    auto implements = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Implements), StructFlags::None);
    implements->addStructField("method1",
                               1,
                               std::string(Project_Entity_Name) + "." + Api_Entity_Name + ".namespace.class.method." +
                                   GetPredefinedStructName(StructTypeId::Method_Desc),
                               FieldFlags::Repeated,
                               "",
                               std::move(implementedMethodDesc));

    auto invokes = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Invokes));
    invokes->addStructField("method1",
                            1,
                            std::string(Project_Entity_Name) + "." + Api_Entity_Name + ".namespace.class.method." +
                                GetPredefinedStructName(StructTypeId::Method_Desc),
                            FieldFlags::None,
                            "oneofName",
                            EntityDocs("Invoked method 1."));

    return desc;
}

Service* AddService(Services* services)
{
    auto service = services->addService("service");
    AddServiceDesc(service);
    AddStructsAndEnums(service);
    return service;
}

void InitApi(Api* api)
{
    AddErrc(api);
    AddException(api);
    AddCallMessage(api);
    AddResultMessage(api);
    AddStructsAndEnums(api);
}

void InitServices(Services* services)
{
    AddService(services);
    AddStructsAndEnums(services);
}

void InitProject(Project* project)
{
    auto api = project->addApi();
    InitApi(api);
    AddMethod(AddClass(AddNamespace(api)));
    InitServices(project->addServices());
}

class ProjectCheckTest: public ::testing::Test {
protected:
    void SetUp() override
    {
        api_ = project_.addApi();
        InitApi(api_);
        services_ = project_.addServices();
    }

protected:
    Project project_;
    Api* api_ = nullptr;
    Services* services_ = nullptr;
};

TEST_F(ProjectCheckTest, Specification_Error_Category_Name_Is_Not_Empty)
{
    EXPECT_TRUE(spec_error_category().name());
    EXPECT_NE(spec_error_category().name()[0], 0);
}

TEST_F(ProjectCheckTest, Specification_Error_Codes_Have_Non_Empty_Descriptions)
{
    using enum SpecErrc;

    EXPECT_FALSE(spec_error_category().message(static_cast<int>(Missing_Api)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(Missing_Builtin)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(Nonconforming_Builtin)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(Missing_Descriptor)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(Not_Static_Method)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(Not_Encodable_Type)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(Not_Accessible_Type)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(Unknown_Type)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(Unexpected_Type)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(Unknown_Method)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(Multiple_Definitions)).empty());
}

TEST_F(ProjectCheckTest, Unknown_Specification_Error_Code_Has_Non_Empty_Description)
{
    EXPECT_FALSE(spec_error_category().message(0).empty());
}

TEST_F(ProjectCheckTest, Specification_Warning_Category_Name_Is_Not_Empty)
{
    EXPECT_TRUE(spec_warn_category().name());
    EXPECT_NE(spec_warn_category().name()[0], 0);
}

TEST_F(ProjectCheckTest, Specification_Warning_Codes_Have_Non_Empty_Descriptions)
{
    using enum SpecWarn;

    EXPECT_FALSE(spec_warn_category().message(static_cast<int>(Unexpected_Nested_Entity)).empty());
}

TEST_F(ProjectCheckTest, Unknown_Specification_Warning_Code_Has_Non_Empty_Description)
{
    EXPECT_FALSE(spec_warn_category().message(0).empty());
}

TEST_F(ProjectCheckTest, Documentation_Error_Category_Name_Is_Not_Empty)
{
    EXPECT_TRUE(doc_error_category().name());
    EXPECT_NE(doc_error_category().name()[0], 0);
}

TEST_F(ProjectCheckTest, Documentation_Error_Codes_Have_Non_Empty_Descriptions)
{
    using enum DocErrc;

    EXPECT_FALSE(doc_error_category().message(static_cast<int>(Undocumented_Entity)).empty());
    EXPECT_FALSE(doc_error_category().message(static_cast<int>(Unknown_Doc_Command)).empty());
}

TEST_F(ProjectCheckTest, Unknown_Documentation_Error_Code_Has_Non_Empty_Description)
{
    EXPECT_FALSE(doc_error_category().message(0).empty());
}

TEST_F(ProjectCheckTest, Style_Error_Category_Name_Is_Not_Empty)
{
    EXPECT_TRUE(style_error_category().name());
    EXPECT_NE(style_error_category().name()[0], 0);
}

TEST_F(ProjectCheckTest, Style_Error_Codes_Have_Non_Empty_Descriptions)
{
    using enum StyleErrc;

    EXPECT_FALSE(style_error_category().message(static_cast<int>(Invalid_Name_Format)).empty());
}

TEST_F(ProjectCheckTest, Unknown_Style_Error_Code_Has_Non_Empty_Description)
{
    EXPECT_FALSE(style_error_category().message(0).empty());
}

TEST_F(ProjectCheckTest, Success_For_Conforming_Project)
{
    Project project;
    InitProject(&project);
    ErrorCollector ecol = project.check();

    EXPECT_FALSE(ecol);
    EXPECT_EQ(ecol.errors().size(), 0);

    for (const auto& error: ecol.errors()) {
        std::cout << "INFO: " << error.description << "\n";
    }
}

TEST_F(ProjectCheckTest, Missing_Api_Spec_Error_If_Api_Is_Not_Initialized)
{
    Project project;
    ErrorCollector ecol = project.check();

    ASSERT_TRUE(ecol);
    EXPECT_EQ(ecol.errors().size(), 1);
    EXPECT_EQ(ecol.majorError()->code, SpecErrc::Missing_Api);
}

TEST_F(ProjectCheckTest, Missing_Builtin_Spec_Error_If_Errc_Is_Not_Added)
{
    Project project;
    auto api = project.addApi();
    AddException(api);
    AddCallMessage(api);
    AddResultMessage(api);

    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Builtin));
}

TEST_F(ProjectCheckTest, Missing_Builtin_Spec_Error_If_Errc_Is_Defined_In_Unexpected_File)
{
    Project project;
    auto api = project.addApi();
    AddException(api);
    AddCallMessage(api);
    AddResultMessage(api);

    api->addEnum(Errc_Enum_Name, "1.proto");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Builtin));
}

TEST_F(ProjectCheckTest, Missing_Builtin_Spec_Error_If_Exception_Is_Not_Added)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddCallMessage(api);
    AddResultMessage(api);

    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Builtin));
}

TEST_F(ProjectCheckTest, Missing_Builtin_Spec_Error_If_Exception_Is_Defined_In_Unexpected_File)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddCallMessage(api);
    AddResultMessage(api);

    auto exception = api->addStruct(GetPredefinedStructName(StructTypeId::Method_Exception), "1.proto");
    exception->addEnumField(Exception_Code_Field_Name, 1, api->dname() + ".Errc");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Exception_Does_Not_Have_Code_Field)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddCallMessage(api);
    AddResultMessage(api);

    api->addStruct("Exception", Busrpc_Builtin_File, StructFlags::None);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Exception_Code_Field_Is_Not_Errc)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddCallMessage(api);
    AddResultMessage(api);

    auto exception =
        api->addStruct(GetPredefinedStructName(StructTypeId::Method_Exception), Busrpc_Builtin_File, StructFlags::None);
    exception->addScalarField(Exception_Code_Field_Name, 1, FieldTypeId::Int32);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Exception_Code_Field_Is_Optional)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddCallMessage(api);
    AddResultMessage(api);

    auto exception = api->addStruct(GetPredefinedStructName(StructTypeId::Method_Exception), Busrpc_Builtin_File);
    exception->addEnumField(Exception_Code_Field_Name, 1, api->dname() + ".Errc", FieldFlags::Optional);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Exception_Code_Field_Is_Repeated)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddCallMessage(api);
    AddResultMessage(api);

    auto exception = api->addStruct(GetPredefinedStructName(StructTypeId::Method_Exception), Busrpc_Builtin_File);
    exception->addEnumField(Exception_Code_Field_Name, 1, api->dname() + ".Errc", FieldFlags::Repeated);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Missing_Builtin_Spec_Error_If_Call_Message_Is_Not_Added)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddException(api);
    AddResultMessage(api);

    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Builtin));
}

TEST_F(ProjectCheckTest, Missing_Builtin_Spec_Error_If_Call_Message_Is_Defined_In_Unexpected_File)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddException(api);
    AddResultMessage(api);

    auto call = api->addStruct(GetPredefinedStructName(StructTypeId::Call_Message), "1.proto");
    call->addScalarField(Call_Message_Object_Id_Field_Name, 5, FieldTypeId::Bytes, FieldFlags::Optional);
    call->addScalarField(Call_Message_Params_Field_Name, 6, FieldTypeId::Bytes, FieldFlags::Optional);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Call_Message_Does_Not_Have_Object_Id_Field)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddException(api);
    AddResultMessage(api);

    auto call = api->addStruct(GetPredefinedStructName(StructTypeId::Call_Message), Busrpc_Builtin_File);
    call->addScalarField(Call_Message_Params_Field_Name, 6, FieldTypeId::Bytes, FieldFlags::Optional);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Call_Message_Does_Not_Have_Params_Field)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddException(api);
    AddResultMessage(api);

    auto call = api->addStruct(GetPredefinedStructName(StructTypeId::Call_Message), Busrpc_Builtin_File);
    call->addScalarField(Call_Message_Object_Id_Field_Name, 5, FieldTypeId::Bytes, FieldFlags::Optional);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Call_Message_Object_Id_Field_Is_Not_Bytes)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddException(api);
    AddResultMessage(api);

    auto call = api->addStruct(GetPredefinedStructName(StructTypeId::Call_Message), Busrpc_Builtin_File);
    call->addScalarField(Call_Message_Object_Id_Field_Name, 5, FieldTypeId::Int32, FieldFlags::Optional);
    call->addScalarField(Call_Message_Params_Field_Name, 6, FieldTypeId::Bytes, FieldFlags::Optional);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Call_Message_Params_Field_Is_Not_Bytes)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddException(api);
    AddResultMessage(api);

    auto call = api->addStruct(GetPredefinedStructName(StructTypeId::Call_Message), Busrpc_Builtin_File);
    call->addScalarField(Call_Message_Object_Id_Field_Name, 5, FieldTypeId::Bytes, FieldFlags::Optional);
    call->addScalarField(Call_Message_Params_Field_Name, 6, FieldTypeId::Int32, FieldFlags::Optional);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Call_Message_Object_Id_Field_Is_Not_Optional)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddException(api);
    AddResultMessage(api);

    auto call = api->addStruct(GetPredefinedStructName(StructTypeId::Call_Message), Busrpc_Builtin_File);
    call->addScalarField(Call_Message_Object_Id_Field_Name, 5, FieldTypeId::Bytes);
    call->addScalarField(Call_Message_Params_Field_Name, 6, FieldTypeId::Bytes, FieldFlags::Optional);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Call_Message_Params_Field_Is_Not_Optional)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddException(api);
    AddResultMessage(api);

    auto call = api->addStruct(GetPredefinedStructName(StructTypeId::Call_Message), Busrpc_Builtin_File);
    call->addScalarField(Call_Message_Object_Id_Field_Name, 5, FieldTypeId::Bytes, FieldFlags::Optional);
    call->addScalarField(Call_Message_Params_Field_Name, 6, FieldTypeId::Bytes);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Call_Message_Has_Unknown_Fields)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddException(api);
    AddResultMessage(api);

    auto call = AddCallMessage(api);
    call->addScalarField("unknown_field", 7, FieldTypeId::Bytes, FieldFlags::Optional);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Missing_Builtin_Spec_Error_If_Result_Message_Is_Not_Added)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddException(api);
    AddCallMessage(api);

    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Builtin));
}

TEST_F(ProjectCheckTest, Missing_Builtin_Spec_Error_If_Result_Message_Is_Defined_In_Unexpected_File)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddException(api);
    AddCallMessage(api);

    auto result = api->addStruct(GetPredefinedStructName(StructTypeId::Result_Message), "1.proto");
    result->addScalarField(Result_Message_Retval_Field_Name, 5, FieldTypeId::Bytes, FieldFlags::None, "Result");
    result->addStructField(
        Result_Message_Exception_Field_Name, 6, api->dname() + ".Exception", FieldFlags::None, "Result");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Result_Message_Does_Not_Have_Retval_Field)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddException(api);
    AddCallMessage(api);

    auto result = api->addStruct(GetPredefinedStructName(StructTypeId::Result_Message), Busrpc_Builtin_File);
    result->addStructField(
        Result_Message_Exception_Field_Name, 6, api->dname() + ".Exception", FieldFlags::None, "Result");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Result_Message_Does_Not_Have_Exception_Field)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddException(api);
    AddCallMessage(api);

    auto result = api->addStruct(GetPredefinedStructName(StructTypeId::Result_Message), Busrpc_Builtin_File);
    result->addScalarField(Result_Message_Retval_Field_Name, 5, FieldTypeId::Bytes, FieldFlags::None, "Result");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Result_Message_Retval_Field_Is_Not_Bytes)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddException(api);
    AddCallMessage(api);

    auto result = api->addStruct(GetPredefinedStructName(StructTypeId::Result_Message), Busrpc_Builtin_File);
    result->addScalarField(Result_Message_Retval_Field_Name, 5, FieldTypeId::Int32, FieldFlags::None, "Result");
    result->addStructField(
        Result_Message_Exception_Field_Name, 6, api->dname() + ".Exception", FieldFlags::None, "Result");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Result_Message_Exception_Field_Is_Not_Exception)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddException(api);
    AddCallMessage(api);

    auto result = api->addStruct(GetPredefinedStructName(StructTypeId::Result_Message), Busrpc_Builtin_File);
    result->addScalarField(Result_Message_Retval_Field_Name, 5, FieldTypeId::Bytes, FieldFlags::None, "Result");
    result->addScalarField(Result_Message_Exception_Field_Name, 6, FieldTypeId::Int32, FieldFlags::None, "Result");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Result_Message_Retval_Field_Is_Not_Part_Of_Oneof)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddException(api);
    AddCallMessage(api);

    auto result = api->addStruct(GetPredefinedStructName(StructTypeId::Result_Message), Busrpc_Builtin_File);
    result->addScalarField(Result_Message_Retval_Field_Name, 5, FieldTypeId::Bytes, FieldFlags::None);
    result->addStructField(
        Result_Message_Exception_Field_Name, 6, api->dname() + ".Exception", FieldFlags::None, "Result");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Result_Message_Exception_Field_Is_Not_Part_Of_Oneof)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddException(api);
    AddCallMessage(api);

    auto result = api->addStruct(GetPredefinedStructName(StructTypeId::Result_Message), Busrpc_Builtin_File);
    result->addScalarField(Result_Message_Retval_Field_Name, 5, FieldTypeId::Bytes, FieldFlags::None, "Result");
    result->addStructField(Result_Message_Exception_Field_Name, 6, api->dname() + ".Exception", FieldFlags::None);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest,
       Nonconforming_Builtin_Spec_Error_If_Result_Message_Retval_And_Exception_Fields_Does_Not_Belong_To_Same_Oneof)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddException(api);
    AddCallMessage(api);

    auto result = api->addStruct(GetPredefinedStructName(StructTypeId::Result_Message), Busrpc_Builtin_File);
    result->addScalarField(Result_Message_Retval_Field_Name, 5, FieldTypeId::Bytes, FieldFlags::None, "Result");
    result->addStructField(
        Result_Message_Exception_Field_Name, 6, api->dname() + ".Exception", FieldFlags::None, "Result1");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Result_Message_Has_Unknown_Fields)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddException(api);
    AddCallMessage(api);

    auto result = AddResultMessage(api);
    result->addScalarField("unknown_field", 7, FieldTypeId::Int32);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Missing_Descriptor_Spec_Error_If_Namespace_Does_Not_Have_Descriptor)
{
    api_->addNamespace("namespace");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Descriptor));
}

TEST_F(ProjectCheckTest, Missing_Descriptor_Spec_Error_If_Namespace_Descriptor_Is_Defined_In_Unexpected_File)
{
    auto ns = api_->addNamespace("namespace");
    ns->addStruct(GetPredefinedStructName(StructTypeId::Namespace_Desc), "1.proto");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Descriptor));
}

TEST_F(ProjectCheckTest, Unexpected_Nested_Entity_Spec_Warn_If_Namespace_Descriptor_Has_Unexpected_Nested_Struct)
{
    auto ns = api_->addNamespace("namespace");
    AddNamespaceDesc(ns)->addStruct("NestedStruct", StructFlags::None, EntityDocs("Nested struct."));
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST_F(ProjectCheckTest, Unexpected_Nested_Entity_Spec_Warn_If_Namespace_Descriptor_Has_Unexpected_Nested_Enum)
{
    auto ns = api_->addNamespace("namespace");
    AddNamespaceDesc(ns)->addEnum("NestedEnum", EntityDocs("Nested enum."));
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST_F(ProjectCheckTest, Unexpected_Nested_Entity_Spec_Warn_If_Namespace_Descriptor_Has_Unexpected_Fields)
{
    auto ns = api_->addNamespace("namespace");
    AddNamespaceDesc(ns)->addScalarField(
        "field", 1, FieldTypeId::Int32, FieldFlags::None, "", "", EntityDocs("Field."));
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST_F(ProjectCheckTest, Invalid_Name_Format_Style_Error_If_Namespace_Name_Is_Not_Lowercase_With_Underscores)
{
    auto ns = api_->addNamespace("Namespace");
    AddNamespaceDesc(ns);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(StyleErrc::Invalid_Name_Format));
}

TEST_F(ProjectCheckTest, Missing_Descriptor_Spec_Error_If_Class_Does_Not_Have_Descriptor)
{
    AddNamespace(api_)->addClass("class");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Descriptor));
}

TEST_F(ProjectCheckTest, Missing_Descriptor_Spec_Error_If_Class_Descriptor_Is_Defined_In_Unexpected_File)
{
    auto cls = AddNamespace(api_)->addClass("class");
    cls->addStruct(GetPredefinedStructName(StructTypeId::Class_Desc), "1.proto");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Descriptor));
}

TEST_F(ProjectCheckTest, Not_Encodable_Type_Spec_Error_If_Class_Object_Id_Is_Not_Encodable)
{
    auto cls = AddNamespace(api_)->addClass("class");
    auto desc = cls->addStruct(GetPredefinedStructName(StructTypeId::Class_Desc), "1.proto");
    auto oid = desc->addStruct(GetPredefinedStructName(StructTypeId::Object_Id));
    oid->addScalarField("field1", 1, FieldTypeId::Int32, FieldFlags::Repeated);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Not_Encodable_Type));
}

TEST_F(ProjectCheckTest, Unexpected_Nested_Entity_Spec_Warn_If_Class_Descriptor_Has_Unexpected_Nested_Struct)
{
    auto cls = AddNamespace(api_)->addClass("class");
    AddClassDesc(cls)->addStruct("NestedStruct", StructFlags::None, EntityDocs("Nested struct."));
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST_F(ProjectCheckTest, Unexpected_Nested_Entity_Spec_Warn_If_Class_Descriptor_Has_Unexpected_Nested_Enum)
{
    auto cls = AddNamespace(api_)->addClass("class");
    AddClassDesc(cls)->addEnum("NestedEnum", EntityDocs("Nested enum."));
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST_F(ProjectCheckTest, Unexpected_Nested_Entity_Spec_Warn_If_Class_Descriptor_Has_Unexpected_Fields)
{
    auto cls = AddNamespace(api_)->addClass("class");
    AddClassDesc(cls)->addScalarField("field", 1, FieldTypeId::Int32, FieldFlags::None, "", "", EntityDocs("Field."));
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST_F(ProjectCheckTest, Invalid_Name_Format_Style_Error_If_Class_Name_Is_Not_Lowercase_With_Underscores)
{
    auto cls = AddNamespace(api_)->addClass("clasS");
    AddClassDesc(cls);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(StyleErrc::Invalid_Name_Format));
}

TEST_F(ProjectCheckTest, Missing_Descriptor_Spec_Error_If_Method_Does_Not_Have_Descriptor)
{
    AddClass(AddNamespace(api_))->addMethod("method");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Descriptor));
}

TEST_F(ProjectCheckTest, Missing_Descriptor_Spec_Error_If_Method_Descriptor_Is_Defined_In_Unexpected_File)
{
    auto method = AddClass(AddNamespace(api_))->addMethod("method");
    method->addStruct(GetPredefinedStructName(StructTypeId::Method_Desc), "1.proto");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Descriptor));
}

TEST_F(ProjectCheckTest, Not_Static_Method_Spec_Error_If_Non_Static_Method_Is_Added_To_Static_Class)
{
    AddMethod(AddClass(AddNamespace(api_), true), false);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Not_Static_Method));
}

TEST_F(ProjectCheckTest, Unexpected_Nested_Entity_Spec_Warn_If_Method_Descriptor_Has_Unexpected_Nested_Struct)
{
    auto method = AddClass(AddNamespace(api_))->addMethod("method");
    AddMethodDesc(method)->addStruct("NestedStruct", StructFlags::None, EntityDocs("Nested struct."));
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST_F(ProjectCheckTest, Unexpected_Nested_Entity_Spec_Warn_If_Method_Descriptor_Has_Unexpected_Nested_Enum)
{
    auto method = AddClass(AddNamespace(api_))->addMethod("method");
    AddMethodDesc(method)->addEnum("NestedEnum", EntityDocs("Nested enum."));
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST_F(ProjectCheckTest, Unexpected_Nested_Entity_Spec_Warn_If_Method_Descriptor_Has_Unexpected_Fields)
{
    auto method = AddClass(AddNamespace(api_))->addMethod("method");
    AddMethodDesc(method)->addScalarField(
        "field", 1, FieldTypeId::Int32, FieldFlags::None, "", "", EntityDocs("Field."));
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST_F(ProjectCheckTest, Invalid_Name_Format_Style_Error_If_Method_Name_Is_Not_Lowercase_With_Underscores)
{
    auto method = AddClass(AddNamespace(api_))->addMethod("meThod");
    AddMethodDesc(method);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(StyleErrc::Invalid_Name_Format));
}

TEST_F(ProjectCheckTest, Missing_Descriptor_Spec_Error_If_Service_Does_Not_Have_Descriptor)
{
    services_->addService("service");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Descriptor));
}

TEST_F(ProjectCheckTest, Missing_Descriptor_Spec_Error_If_Service_Descriptor_Is_Defined_In_Unexpected_File)
{
    auto service = services_->addService("service");
    service->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), "1.proto");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Descriptor));
}

TEST_F(ProjectCheckTest,
       Unknown_Method_Spec_Error_If_Service_Implements_Contains_Field_Whose_Type_Is_Not_Existing_Method_Descriptor)
{
    auto service = services_->addService("service");
    auto desc = service->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), Service_Desc_File);
    auto implements = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Implements));
    implements->addScalarField("field1", 1, FieldTypeId::Int32);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Unknown_Method));
}

TEST_F(ProjectCheckTest,
       Unknown_Method_Spec_Error_If_Service_Invokes_Contains_Field_Whose_Type_Is_Not_Existing_Method_Descriptor)
{
    auto service = services_->addService("service");
    auto desc = service->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), Service_Desc_File);
    auto invokes = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Invokes));
    invokes->addScalarField("field1", 1, FieldTypeId::Int32);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Unknown_Method));
}

TEST_F(ProjectCheckTest, Multiple_Definitions_Spec_Error_If_Service_Implements_References_Same_Method_More_Than_Once)
{
    auto method = AddMethod(AddClass(AddNamespace(api_)));
    auto service = services_->addService("service");
    auto desc = service->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), Service_Desc_File);

    auto implements = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Implements));
    implements->addStructField("field1", 1, method->descriptor()->dname());
    implements->addStructField("field2", 2, method->descriptor()->dname());

    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Multiple_Definitions));
}

TEST_F(ProjectCheckTest, Multiple_Definitions_Spec_Error_If_Service_Invokes_References_Same_Method_More_Than_Once)
{
    auto method = AddMethod(AddClass(AddNamespace(api_)));
    auto service = services_->addService("service");
    auto desc = service->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), Service_Desc_File);

    auto invokes = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Invokes));
    invokes->addStructField("field1", 1, method->descriptor()->dname());
    invokes->addStructField("field2", 2, method->descriptor()->dname());

    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Multiple_Definitions));
}

TEST_F(ProjectCheckTest, Unexpected_Nested_Entity_Spec_Warn_If_Service_Descriptor_Has_Unexpected_Nested_Struct)
{
    auto service = services_->addService("service");
    AddServiceDesc(service)->addStruct("UnexpectedStruct", StructFlags::None, EntityDocs("Unexpected nested struct."));
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST_F(ProjectCheckTest, Unexpected_Nested_Entity_Spec_Warn_If_Service_Descriptor_Has_Unexpected_Nested_Enum)
{
    auto service = services_->addService("service");
    AddServiceDesc(service)->addEnum("UnexpectedEnum", EntityDocs("Unexpected nested enum."));
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST_F(ProjectCheckTest, Unexpected_Nested_Entity_Spec_Warn_If_Service_Descriptor_Has_Unexpected_Fields)
{
    auto service = services_->addService("service");
    AddServiceDesc(service)->addScalarField(
        "field", 1, FieldTypeId::Int32, FieldFlags::None, "", "", EntityDocs("Field."));
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST_F(ProjectCheckTest, Invalid_Name_Format_Style_Error_If_Service_Name_Is_Not_Lowercase_With_Underscores)
{
    auto service = services_->addService("Service");
    AddServiceDesc(service);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(StyleErrc::Invalid_Name_Format));
}

TEST_F(ProjectCheckTest, Unknown_Type_Spec_Error_If_Struct_Type_Of_The_Field_Is_Unknown)
{
    auto structure = api_->addStruct("MyStruct", "1.proto");
    structure->addStructField("field1", 1, "UnknownStruct");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Unknown_Type));
}

TEST_F(ProjectCheckTest, Unknown_Type_Spec_Error_If_Enum_Type_Of_The_Field_Is_Unknown)
{
    auto structure = api_->addStruct("MyStruct", "1.proto");
    structure->addEnumField("field1", 1, "UnknownEnum");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Unknown_Type));
}

TEST_F(ProjectCheckTest, Unknown_Type_Spec_Error_If_Struct_Value_Type_Of_The_Map_Field_Is_Unknown)
{
    auto structure = api_->addStruct("MyStruct", "1.proto");
    structure->addMapField("field1", 1, FieldTypeId::Int32, FieldTypeId::Message, "UnknownStruct");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Unknown_Type));
}

TEST_F(ProjectCheckTest, Unknown_Type_Spec_Error_If_Enum_Value_Type_Of_The_Map_Field_Is_Unknown)
{
    auto structure = api_->addStruct("MyStruct", "1.proto");
    structure->addMapField("field1", 1, FieldTypeId::Int32, FieldTypeId::Enum, "UnknownEnum");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Unknown_Type));
}

TEST_F(ProjectCheckTest, Unexpected_Type_Spec_Error_If_Struct_Type_Of_The_Field_Is_Not_Struct_Entity)
{
    auto enumeration = api_->addEnum("MyEnum", "1.proto");
    auto structure = api_->addStruct("MyStruct", "1.proto");
    structure->addStructField("field1", 1, enumeration->dname());
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Unexpected_Type));
}

TEST_F(ProjectCheckTest, Unexpected_Type_Spec_Error_If_Enum_Type_Of_The_Field_Is_Not_Enum_Entity)
{
    auto structure1 = api_->addStruct("MyStruct1", "1.proto");
    auto structure2 = api_->addStruct("MyStruct2", "1.proto");
    structure2->addEnumField("field1", 1, structure1->dname());
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Unexpected_Type));
}

TEST_F(ProjectCheckTest, Unexpected_Type_Spec_Error_If_Struct_Value_Type_Of_The_Map_Field_Is_Not_Struct_Entity)
{
    auto enumeration = api_->addEnum("MyEnum", "1.proto");
    auto structure = api_->addStruct("MyStruct", "1.proto");
    structure->addMapField("field1", 1, FieldTypeId::Int32, FieldTypeId::Message, enumeration->dname());
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Unexpected_Type));
}

TEST_F(ProjectCheckTest, Unexpected_Type_Spec_Error_If_Enum_Value_Type_Of_The_Map_Field_Is_Not_Enum_Entity)
{
    auto structure1 = api_->addStruct("MyStruct1", "1.proto");
    auto structure2 = api_->addStruct("MyStruct2", "1.proto");
    structure2->addMapField("field1", 1, FieldTypeId::Int32, FieldTypeId::Enum, structure1->dname());
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Unexpected_Type));
}

TEST_F(ProjectCheckTest, Not_Encodable_Type_Spec_Error_If_Non_Encodable_Structure_Is_Marked_As_Hashed)
{
    auto nonEncodable = api_->addStruct("MyStruct", "1.proto", StructFlags::Hashed);
    nonEncodable->addScalarField("field1", 1, FieldTypeId::Int32, FieldFlags::Repeated);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Not_Encodable_Type));
}

TEST_F(ProjectCheckTest, Not_Encodable_Type_Spec_Error_If_Field_Of_Non_Encodable_Type_Is_Marked_As_Observable)
{
    auto structure = api_->addStruct("MyStruct", "1.proto");
    structure->addScalarField("field1", 1, FieldTypeId::Int32, FieldFlags::Observable, "oneofName");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Not_Encodable_Type));
}

TEST_F(ProjectCheckTest, Not_Encodable_Type_Spec_Error_If_Field_Of_Non_Encodable_Type_Is_Marked_As_Hashed)
{
    auto structure = api_->addStruct("MyStruct", "1.proto");
    structure->addScalarField("field1", 1, FieldTypeId::Int32, FieldFlags::Hashed, "oneofName");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Not_Encodable_Type));
}

TEST_F(ProjectCheckTest, Not_Encodable_Type_Spec_Error_If_Field_Of_Non_Encodable_Structure_Type_Is_Marked_As_Observable)
{
    auto structure1 = api_->addStruct("MyStruct1", "1.proto");
    structure1->addScalarField("field1", 1, FieldTypeId::Double);

    auto structure2 = api_->addStruct("MyStruct2", "2.proto");
    structure2->addStructField("field1", 1, structure1->dname(), FieldFlags::Observable);

    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Not_Encodable_Type));
}

TEST_F(ProjectCheckTest, Not_Encodable_Type_Spec_Error_If_Field_Of_Non_Encodable_Structure_Type_Is_Marked_As_Hashed)
{
    auto structure1 = api_->addStruct("MyStruct1", "1.proto");
    structure1->addScalarField("field1", 1, FieldTypeId::Double);

    auto structure2 = api_->addStruct("MyStruct2", "2.proto");
    structure2->addStructField("field1", 1, structure1->dname(), FieldFlags::Hashed);

    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Not_Encodable_Type));
}

TEST_F(ProjectCheckTest, Not_Encodable_Type_Spec_Error_If_Field_Of_Google_Type_Is_Marked_As_Observable)
{
    auto structure = api_->addStruct("MyStruct", "1.proto");
    structure->addStructField("field1", 1, "google.protobuf.Any", FieldFlags::Observable);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Not_Encodable_Type));
}

TEST_F(ProjectCheckTest, Not_Encodable_Type_Spec_Error_If_Field_Google_Type_Is_Marked_As_Hashed)
{
    auto structure = api_->addStruct("MyStruct", "1.proto");
    structure->addStructField("field1", 1, "google.protobuf.Any", FieldFlags::Hashed);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Not_Encodable_Type));
}

TEST_F(ProjectCheckTest, Not_Accessible_Type_Spec_Error_If_Referenced_Type_Outside_The_Current_Scope)
{
    auto namespaceStruct = AddNamespace(api_)->addStruct("MyStruct", "1.proto");
    auto apiStruct = api_->addStruct("MyStruct", "1.proto");
    apiStruct->addStructField("field1", 1, namespaceStruct->dname());
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Not_Accessible_Type));
}

TEST_F(ProjectCheckTest, Invalid_Name_Format_Style_Error_If_Struct_Name_Is_Not_CamelCase)
{
    api_->addStruct("myStruct", "1.proto");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(StyleErrc::Invalid_Name_Format));
}

TEST_F(ProjectCheckTest, Invalid_Name_Format_Style_Error_If_Enum_Name_Is_Not_CamelCase)
{
    api_->addEnum("MY_ENUM", "1.proto");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(StyleErrc::Invalid_Name_Format));
}

TEST_F(ProjectCheckTest, Invalid_Name_Format_Style_Error_If_Struct_Field_Name_Is_Not_Lowercase_With_Underscores)
{
    auto structure = api_->addStruct("MyStruct", "1.proto");
    structure->addScalarField("MyField", 1, FieldTypeId::Int32);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(StyleErrc::Invalid_Name_Format));
}

TEST_F(ProjectCheckTest, Invalid_Name_Format_Style_Error_If_Enum_Constant_Name_Is_Not_Uppercase_With_Underscores)
{
    auto enumeration = api_->addEnum("MyEnum", "1.proto");
    enumeration->addConstant("TESt_CONSTANT", 1001);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(StyleErrc::Invalid_Name_Format));
}

TEST_F(ProjectCheckTest, Undocumented_Entity_Doc_Error_If_Struct_Is_Not_Documented)
{
    api_->addStruct("MyStruct", "1.proto");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(DocErrc::Undocumented_Entity));
}

TEST_F(ProjectCheckTest, Undocumented_Entity_Doc_Error_If_Enum_Is_Not_Documented)
{
    api_->addEnum("MyEnum", "1.proto");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(DocErrc::Undocumented_Entity));
}

TEST_F(ProjectCheckTest, Undocumented_Entity_Doc_Error_If_Struct_Field_Is_Not_Documented)
{
    auto structure = api_->addStruct("MyStruct", "1.proto", StructFlags::None, EntityDocs("My struct."));
    structure->addScalarField("field1", 1, FieldTypeId::Int32);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(DocErrc::Undocumented_Entity));
}

TEST_F(ProjectCheckTest, Undocumented_Entity_Doc_Error_If_Enum_Constant_Is_Not_Documented)
{
    auto enumeration = api_->addEnum("MyEnum", "1.proto", EntityDocs("My enumeration."));
    enumeration->addConstant("CONSTANT_1", 1);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(DocErrc::Undocumented_Entity));
}

TEST_F(ProjectCheckTest, Unknown_Doc_Command_Doc_Error_If_Struct_Documentation_Command_Is_Unrecognized)
{
    api_->addStruct("MyStruct", "1.proto", StructFlags::None, {{"My structure."}, {{"cmd1", {"value1"}}}});
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(DocErrc::Unknown_Doc_Command));
}

TEST_F(ProjectCheckTest, Unknown_Doc_Command_Doc_Error_If_Enum_Documentation_Command_Is_Unrecognized)
{
    api_->addEnum("MyEnum", "1.proto", {{"My enumeration."}, {{"cmd1", {"value1"}}}});
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(DocErrc::Unknown_Doc_Command));
}

TEST_F(ProjectCheckTest, Unknown_Doc_Command_Doc_Error_If_Struct_Field_Documentation_Command_Is_Unrecognized)
{
    auto structure = api_->addStruct("MyStruct", "1.proto", StructFlags::None, EntityDocs("My structure."));
    structure->addScalarField(
        "field1", 1, FieldTypeId::Int32, FieldFlags::None, "", "", {{"Field 1."}, {{"cmd1", {"value1"}}}});
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(DocErrc::Unknown_Doc_Command));
}

TEST_F(ProjectCheckTest, Unknown_Doc_Command_Doc_Error_If_Enum_Constant_Documentation_Command_Is_Unrecognized)
{
    auto enumeration = api_->addEnum("MyEnum", "1.proto", EntityDocs("My enumeration."));
    enumeration->addConstant("CONSTANT_1", 1001, {{"Constant 1."}, {{"cmd1", {"value1"}}}});
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(DocErrc::Unknown_Doc_Command));
}

TEST_F(ProjectCheckTest, Default_Severity_Of_Errors_Is_SpecErrc_DocErrc_SpecWarn_StyleErrc)
{
    {
        Project project;
        auto api = project.addApi();
        InitApi(api);

        auto ns = api->addNamespace("Namespace"); // non-conformat name (should be lowercase)
        auto desc = ns->addStruct(                // unexpected file (should be 'namespace.proto'), undocumented entity
            GetPredefinedStructName(StructTypeId::Namespace_Desc),
            "1.proto",
            StructFlags::None);
        desc->addStruct("MyStruct", StructFlags::None, EntityDocs("My structure.")); // unexpected nested type

        ErrorCollector ecol = project.check();

        EXPECT_EQ(ecol.majorError()->code.category(), spec_error_category());
    }

    {
        Project project;
        auto api = project.addApi();
        InitApi(api);

        auto ns = api->addNamespace("Namespace"); // non-conformat name (should be lowercase)
        auto desc = ns->addStruct(                // undocumented entity
            GetPredefinedStructName(StructTypeId::Namespace_Desc),
            Namespace_Desc_File,
            StructFlags::None);
        desc->addStruct("MyStruct", StructFlags::None, EntityDocs("My structure.")); // unexpected nested type

        ErrorCollector ecol = project.check();

        EXPECT_EQ(ecol.majorError()->code.category(), doc_error_category());
    }

    {
        Project project;
        auto api = project.addApi();
        InitApi(api);

        auto ns = api->addNamespace("Namespace"); // non-conformat name (should be lowercase)
        auto desc = ns->addStruct(GetPredefinedStructName(StructTypeId::Namespace_Desc),
                                  Namespace_Desc_File,
                                  StructFlags::None,
                                  EntityDocs("Namespace."));
        desc->addStruct("MyStruct", StructFlags::None, EntityDocs("My structure.")); // unexpected nested type

        ErrorCollector ecol = project.check();

        EXPECT_EQ(ecol.majorError()->code.category(), spec_warn_category());
    }
}
}} // namespace busrpc::test
