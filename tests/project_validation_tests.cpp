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
                              "oneofName",
                              "1001",
                              EntityDocs("Field 1."));

    if (parent) {
        topStruct->addMapField("field2",
                               2,
                               FieldTypeId::String,
                               FieldTypeId::Message,
                               parent->dname() + ".Struct",
                               EntityDocs("Field 2."));
        topStruct->addStructField("field3",
                                  3,
                                  parent->dname() + ".Struct.NestedStruct",
                                  FieldFlags::Optional | FieldFlags::Observable | FieldFlags::Hashed,
                                  "",
                                  EntityDocs("Field 3."));
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

TEST(ProjectCheckTest, Specification_Error_Category_Name_Is_Not_Empty)
{
    EXPECT_TRUE(spec_error_category().name());
    EXPECT_NE(spec_error_category().name()[0], 0);
}

TEST(ProjectCheckTest, Specification_Error_Codes_Have_Non_Empty_Descriptions)
{
    using enum SpecErrc;

    EXPECT_FALSE(spec_error_category().message(static_cast<int>(Missing_Api)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(Missing_Builtin)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(Nonconforming_Builtin)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(Missing_Descriptor)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(Not_Static_Method)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(Not_Encodable_Type)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(Not_Accessible_Type)).empty());
}

TEST(ProjectCheckTest, Unknown_Specification_Error_Code_Has_Non_Empty_Description)
{
    EXPECT_FALSE(spec_error_category().message(0).empty());
}

TEST(ProjectCheckTest, Specification_Warning_Category_Name_Is_Not_Empty)
{
    EXPECT_TRUE(spec_warn_category().name());
    EXPECT_NE(spec_warn_category().name()[0], 0);
}

TEST(ProjectCheckTest, Specification_Warning_Codes_Have_Non_Empty_Descriptions)
{
    using enum SpecWarn;

    EXPECT_FALSE(spec_warn_category().message(static_cast<int>(Unexpected_Nested_Entity)).empty());
}

TEST(ProjectCheckTest, Unknown_Specification_Warning_Code_Has_Non_Empty_Description)
{
    EXPECT_FALSE(spec_warn_category().message(0).empty());
}

TEST(ProjectCheckTest, Documentation_Error_Category_Name_Is_Not_Empty)
{
    EXPECT_TRUE(doc_error_category().name());
    EXPECT_NE(doc_error_category().name()[0], 0);
}

TEST(ProjectCheckTest, Documentation_Error_Codes_Have_Non_Empty_Descriptions)
{
    using enum DocErrc;

    EXPECT_FALSE(doc_error_category().message(static_cast<int>(Undocumented_Entity)).empty());
    EXPECT_FALSE(doc_error_category().message(static_cast<int>(Unknown_Doc_Command)).empty());
}

TEST(ProjectCheckTest, Unknown_Documentation_Error_Code_Has_Non_Empty_Description)
{
    EXPECT_FALSE(doc_error_category().message(0).empty());
}

TEST(ProjectCheckTest, Style_Error_Category_Name_Is_Not_Empty)
{
    EXPECT_TRUE(style_error_category().name());
    EXPECT_NE(style_error_category().name()[0], 0);
}

TEST(ProjectCheckTest, Style_Error_Codes_Have_Non_Empty_Descriptions)
{
    using enum StyleErrc;

    EXPECT_FALSE(style_error_category().message(static_cast<int>(Invalid_Name_Format)).empty());
}

TEST(ProjectCheckTest, Unknown_Style_Error_Code_Has_Non_Empty_Description)
{
    EXPECT_FALSE(style_error_category().message(0).empty());
}

TEST(ProjectCheckTest, Success_For_Conforming_Project)
{
    Project project;
    InitProject(&project);
    ErrorCollector ecol = project.check();

    EXPECT_FALSE(ecol);
}

TEST(ProjectCheckTest, Missing_Api_Spec_Error_If_Api_Is_Not_Initialized)
{
    Project project;
    ErrorCollector ecol = project.check();

    ASSERT_TRUE(ecol);
    EXPECT_EQ(ecol.errors().size(), 1);
    EXPECT_EQ(ecol.majorError()->code, SpecErrc::Missing_Api);
}

TEST(ProjectCheckTest, Missing_Builtin_Spec_Error_If_Errc_Is_Not_Added)
{
    Project project;
    auto api = project.addApi();
    AddException(api);
    AddCallMessage(api);
    AddResultMessage(api);

    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Builtin));
}

TEST(ProjectCheckTest, Missing_Builtin_Spec_Error_If_Errc_Is_Defined_In_Unexpected_File)
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

TEST(ProjectCheckTest, Missing_Builtin_Spec_Error_If_Exception_Is_Not_Added)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddCallMessage(api);
    AddResultMessage(api);

    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Builtin));
}

TEST(ProjectCheckTest, Missing_Builtin_Spec_Error_If_Exception_Is_Defined_In_Unexpected_File)
{
    Project project;
    auto api = project.addApi();
    AddException(api);
    AddCallMessage(api);
    AddResultMessage(api);

    auto exception = api->addStruct(GetPredefinedStructName(StructTypeId::Method_Exception), "1.proto");
    exception->addEnumField(Exception_Code_Field_Name, 1, api->dname() + ".Errc");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Builtin));
}

TEST(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Exception_Does_Not_Have_Code_Field)
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

TEST(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Exception_Code_Field_Is_Not_Errc)
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

TEST(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Exception_Code_Field_Is_Optional)
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

TEST(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Exception_Code_Field_Is_Repeated)
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

TEST(ProjectCheckTest, Missing_Builtin_Spec_Error_If_Call_Message_Is_Not_Added)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddException(api);
    AddResultMessage(api);

    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Builtin));
}

TEST(ProjectCheckTest, Missing_Builtin_Spec_Error_If_Call_Message_Is_Defined_In_Unexpected_File)
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

TEST(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Call_Message_Does_Not_Have_Object_Id_Field)
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

TEST(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Call_Message_Does_Not_Have_Params_Field)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddException(api);
    AddResultMessage(api);

    auto call = api->addStruct(GetPredefinedStructName(StructTypeId::Call_Message), "1.proto");
    call->addScalarField(Call_Message_Object_Id_Field_Name, 5, FieldTypeId::Bytes, FieldFlags::Optional);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Call_Message_Object_Id_Field_Is_Not_Bytes)
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

TEST(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Call_Message_Params_Field_Is_Not_Bytes)
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

TEST(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Call_Message_Object_Id_Field_Is_Not_Optional)
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

TEST(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Call_Message_Params_Field_Is_Not_Optional)
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

TEST(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Call_Message_Object_Id_Field_Is_Part_Of_Oneof)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddException(api);
    AddResultMessage(api);

    auto call = api->addStruct(GetPredefinedStructName(StructTypeId::Call_Message), Busrpc_Builtin_File);
    call->addScalarField(Call_Message_Object_Id_Field_Name, 5, FieldTypeId::Bytes, FieldFlags::Optional, "oneofName");
    call->addScalarField(Call_Message_Params_Field_Name, 6, FieldTypeId::Bytes, FieldFlags::Optional);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Call_Message_Params_Field_Is_Part_Of_Oneof)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddException(api);
    AddResultMessage(api);

    auto call = api->addStruct(GetPredefinedStructName(StructTypeId::Call_Message), Busrpc_Builtin_File);
    call->addScalarField(Call_Message_Object_Id_Field_Name, 5, FieldTypeId::Bytes, FieldFlags::Optional);
    call->addScalarField(Call_Message_Params_Field_Name, 6, FieldTypeId::Bytes, FieldFlags::Optional, "oneofName");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Call_Message_Object_Id_Field_Is_Repeated)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddException(api);
    AddResultMessage(api);

    auto call = api->addStruct(GetPredefinedStructName(StructTypeId::Call_Message), Busrpc_Builtin_File);
    call->addScalarField(
        Call_Message_Object_Id_Field_Name, 5, FieldTypeId::Bytes, FieldFlags::Optional | FieldFlags::Repeated);
    call->addScalarField(Call_Message_Params_Field_Name, 6, FieldTypeId::Bytes, FieldFlags::Optional);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Call_Message_Params_Field_Is_Repeated)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddException(api);
    AddResultMessage(api);

    auto call = api->addStruct(GetPredefinedStructName(StructTypeId::Call_Message), Busrpc_Builtin_File);
    call->addScalarField(Call_Message_Object_Id_Field_Name, 5, FieldTypeId::Bytes, FieldFlags::Optional);
    call->addScalarField(
        Call_Message_Params_Field_Name, 6, FieldTypeId::Bytes, FieldFlags::Optional | FieldFlags::Repeated);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Call_Message_Has_Unknown_Fields)
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

TEST(ProjectCheckTest, Missing_Builtin_Message_Spec_Error_If_Result_Message_Is_Not_Added)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddException(api);
    AddCallMessage(api);

    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Builtin));
}

TEST(ProjectCheckTest, Missing_Builtin_Message_Spec_Error_If_Result_Message_Is_Defined_In_Unexpected_File)
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

TEST(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Result_Message_Does_Not_Have_Retval_Field)
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

TEST(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Result_Message_Does_Not_Have_Exception_Field)
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

TEST(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Result_Message_Retval_Field_Is_Not_Bytes)
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

TEST(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Result_Message_Exception_Field_Is_Not_Exception)
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

TEST(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Result_Message_Retval_Field_Is_Not_Part_Of_Oneof)
{
    Project project;
    auto api = project.addApi();
    AddErrc(api);
    AddException(api);
    AddCallMessage(api);

    auto result = api->addStruct(GetPredefinedStructName(StructTypeId::Result_Message), Busrpc_Builtin_File);
    result->addScalarField(Result_Message_Retval_Field_Name, 5, FieldTypeId::Bytes, FieldFlags::None);
    result->addStructField(
        Result_Message_Retval_Field_Name, 6, api->dname() + ".Exception", FieldFlags::None, "Result");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Result_Message_Exception_Field_Is_Not_Part_Of_Oneof)
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

TEST(ProjectCheckTest,
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

TEST(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Result_Message_Has_Unknown_Fields)
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

TEST(ProjectCheckTest, Missing_Descriptor_Spec_Error_If_Namespace_Does_Not_Have_Descriptor)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    api->addNamespace("namespace");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Descriptor));
}

TEST(ProjectCheckTest, Missing_Descriptor_Spec_Error_If_Namespace_Descriptor_Is_Defined_In_Unexpected_File)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    auto ns = api->addNamespace("namespace");
    ns->addStruct(GetPredefinedStructName(StructTypeId::Namespace_Desc), "1.proto");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Descriptor));
}

TEST(ProjectCheckTest, Unexpected_Nested_Entity_Spec_Warn_If_Namespace_Descriptor_Has_Unexpected_Nested_Struct)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    auto ns = api->addNamespace("namespace");
    AddNamespaceDesc(ns)->addStruct("nested", StructFlags::None, EntityDocs("Nested struct."));
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST(ProjectCheckTest, Unexpected_Nested_Entity_Spec_Warn_If_Namespace_Descriptor_Has_Unexpected_Nested_Enum)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    auto ns = api->addNamespace("namespace");
    AddNamespaceDesc(ns)->addEnum("nested", EntityDocs("Nested enum."));
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST(ProjectCheckTest, Unexpected_Nested_Entity_Spec_Warn_If_Namespace_Descriptor_Has_Unexpected_Fields)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    auto ns = api->addNamespace("namespace");
    AddNamespaceDesc(ns)->addScalarField(
        "field", 1, FieldTypeId::Int32, FieldFlags::None, "", "", EntityDocs("Field."));
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST(ProjectCheckTest, Invalid_Name_Format_Style_Error_If_Namespace_Name_Is_Not_Lowercase_With_Underscores)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    auto ns = api->addNamespace("Namespace");
    AddNamespaceDesc(ns);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(StyleErrc::Invalid_Name_Format));
}

TEST(ProjectCheckTest, Missing_Descriptor_Spec_Error_If_Class_Does_Not_Have_Descriptor)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    AddNamespace(api)->addClass("class");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Descriptor));
}

TEST(ProjectCheckTest, Missing_Descriptor_Spec_Error_If_Class_Descriptor_Is_Defined_In_Unexpected_File)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    auto cls = AddNamespace(api)->addClass("class");
    cls->addStruct(GetPredefinedStructName(StructTypeId::Class_Desc), "1.proto");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Descriptor));
}

TEST(ProjectCheckTest, Not_Encodable_Type_Spec_Error_If_Class_Object_Id_Is_Not_Encodable)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    auto cls = AddNamespace(api)->addClass("class");
    auto desc = cls->addStruct(GetPredefinedStructName(StructTypeId::Class_Desc), "1.proto");
    auto oid = desc->addStruct(GetPredefinedStructName(StructTypeId::Object_Id));
    oid->addScalarField("field1", 1, FieldTypeId::Int32, FieldFlags::Repeated);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Not_Encodable_Type));
}

TEST(ProjectCheckTest, Unexpected_Nested_Entity_Spec_Warn_If_Class_Descriptor_Has_Unexpected_Nested_Struct)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    auto cls = AddNamespace(api)->addClass("class");
    AddClassDesc(cls)->addStruct("nested", StructFlags::None, EntityDocs("Nested struct."));
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST(ProjectCheckTest, Unexpected_Nested_Entity_Spec_Warn_If_Class_Descriptor_Has_Unexpected_Nested_Enum)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    auto cls = AddNamespace(api)->addClass("class");
    AddClassDesc(cls)->addEnum("nested", EntityDocs("Nested enum."));
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST(ProjectCheckTest, Unexpected_Nested_Entity_Spec_Warn_If_Class_Descriptor_Has_Unexpected_Fields)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    auto cls = AddNamespace(api)->addClass("class");
    AddClassDesc(cls)->addScalarField("field", 1, FieldTypeId::Int32, FieldFlags::None, "", "", EntityDocs("Field."));
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST(ProjectCheckTest, Invalid_Name_Format_Style_Error_If_Class_Name_Is_Not_Lowercase_With_Underscores)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    auto cls = AddNamespace(api)->addClass("clasS");
    AddClassDesc(cls);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(StyleErrc::Invalid_Name_Format));
}

TEST(ProjectCheckTest, Missing_Descriptor_Spec_Error_If_Method_Does_Not_Have_Descriptor)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    AddClass(AddNamespace(api))->addMethod("method");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Descriptor));
}

TEST(ProjectCheckTest, Missing_Descriptor_Spec_Error_If_Method_Descriptor_Is_Defined_In_Unexpected_File)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    auto method = AddClass(AddNamespace(api))->addMethod("method");
    method->addStruct(GetPredefinedStructName(StructTypeId::Method_Desc), "1.proto");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Descriptor));
}

TEST(ProjectCheckTest, Not_Static_Method_Spec_Error_If_Non_Static_Method_Is_Added_To_Static_Class)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);
    AddMethod(AddClass(AddNamespace(api), true), false);

    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Not_Static_Method));
}

TEST(ProjectCheckTest, Unexpected_Nested_Entity_Spec_Warn_If_Method_Descriptor_Has_Unexpected_Nested_Struct)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    auto method = AddClass(AddNamespace(api))->addMethod("method");
    AddMethodDesc(method)->addStruct("nested", StructFlags::None, EntityDocs("Nested struct."));
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST(ProjectCheckTest, Unexpected_Nested_Entity_Spec_Warn_If_Method_Descriptor_Has_Unexpected_Nested_Enum)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    auto method = AddClass(AddNamespace(api))->addMethod("method");
    AddMethodDesc(method)->addEnum("nested", EntityDocs("Nested enum."));
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST(ProjectCheckTest, Unexpected_Nested_Entity_Spec_Warn_If_Method_Descriptor_Has_Unexpected_Fields)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    auto method = AddClass(AddNamespace(api))->addMethod("method");
    AddMethodDesc(method)->addScalarField(
        "field", 1, FieldTypeId::Int32, FieldFlags::None, "", "", EntityDocs("Field."));
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST(ProjectCheckTest, Invalid_Name_Format_Style_Error_If_Method_Name_Is_Not_Lowercase_With_Underscores)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    auto method = AddClass(AddNamespace(api))->addMethod("meThod");
    AddMethodDesc(method);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(StyleErrc::Invalid_Name_Format));
}

TEST(ProjectCheckTest, Missing_Descriptor_Spec_Error_If_Service_Does_Not_Have_Descriptor)
{
    Project project;
    auto api = project.addApi();
    auto services = project.addServices();
    InitApi(api);

    services->addService("service");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Descriptor));
}

TEST(ProjectCheckTest, Missing_Descriptor_Spec_Error_If_Service_Descriptor_Is_Defined_In_Unexpected_File)
{
    Project project;
    auto api = project.addApi();
    auto services = project.addServices();
    InitApi(api);

    auto service = services->addService("service");
    service->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), "1.proto");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Descriptor));
}

TEST(ProjectCheckTest, Unexpected_Nested_Entity_Spec_Warn_If_Service_Descriptor_Has_Unexpected_Nested_Struct)
{
    Project project;
    auto api = project.addApi();
    auto services = project.addServices();
    InitApi(api);

    auto service = services->addService("service");
    AddServiceDesc(service)->addStruct("UnexpectedStruct", StructFlags::None, EntityDocs("Unexpected nested struct."));
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST(ProjectCheckTest, Unexpected_Nested_Entity_Spec_Warn_If_Service_Descriptor_Has_Unexpected_Nested_Enum)
{
    Project project;
    auto api = project.addApi();
    auto services = project.addServices();
    InitApi(api);

    auto service = services->addService("service");
    AddServiceDesc(service)->addEnum("UnexpectedEnum", EntityDocs("Unexpected nested enum."));
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST(ProjectCheckTest, Unexpected_Nested_Entity_Spec_Warn_If_Service_Descriptor_Has_Unexpected_Fields)
{
    Project project;
    auto api = project.addApi();
    auto services = project.addServices();
    InitApi(api);

    auto service = services->addService("service");
    AddServiceDesc(service)->addScalarField(
        "field", 1, FieldTypeId::Int32, FieldFlags::None, "", "", EntityDocs("Field."));
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST(ProjectCheckTest,
     Unexpected_Nested_Entity_Spec_Warn_If_Service_Implements_Contains_Field_Whose_Type_Is_Not_Method_Descriptor)
{
    Project project;
    auto api = project.addApi();
    auto services = project.addServices();
    InitApi(api);

    auto service = services->addService("service");
    auto desc = service->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), Service_Desc_File);
    auto implements = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Implements));
    implements->addScalarField("field1", 1, FieldTypeId::Int32);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST(ProjectCheckTest,
     Unexpected_Nested_Entity_Spec_Warn_If_Service_Invokes_Contains_Field_Whose_Type_Is_Not_Method_Descriptor)
{
    Project project;
    auto api = project.addApi();
    auto services = project.addServices();
    InitApi(api);

    auto service = services->addService("service");
    auto desc = service->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), Service_Desc_File);
    auto invokes = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Invokes));
    invokes->addScalarField("field1", 1, FieldTypeId::Int32);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST(ProjectCheckTest, Invalid_Name_Format_Style_Error_If_Service_Name_Is_Not_Lowercase_With_Underscores)
{
    Project project;
    auto api = project.addApi();
    auto services = project.addServices();
    InitApi(api);

    auto service = services->addService("Service");
    AddServiceDesc(service);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(StyleErrc::Invalid_Name_Format));
}

TEST(ProjectCheckTest, Not_Encodable_Type_Spec_Error_If_Non_Encodable_Structure_Is_Marked_As_Hashed)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    auto nonEncodable = api->addStruct("MyStruct", "1.proto", StructFlags::Hashed);
    nonEncodable->addScalarField("field1", 1, FieldTypeId::Int32, FieldFlags::Repeated);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Not_Encodable_Type));
}

TEST(ProjectCheckTest, Not_Encodable_Type_Spec_Error_If_Non_Encodable_Field_Is_Marked_As_Hashed)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    auto structure = api->addStruct("MyStruct", "1.proto");
    structure->addMapField("field1", 1, FieldTypeId::Int32, FieldTypeId::Int32);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Not_Encodable_Type));
}

TEST(ProjectCheckTest, Not_Encodable_Type_Spec_Error_If_Non_Encodable_Field_Is_Marked_As_Observable)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    auto structure = api->addStruct("MyStruct", "1.proto");
    structure->addScalarField("field1", 1, FieldTypeId::Int32, FieldFlags::Observable, "oneofName");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Not_Encodable_Type));
}

TEST(ProjectCheckTest, Not_Accessible_Type_Spec_Error_If_Referenced_Type_Outside_The_Current_Scope)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    auto namespaceStruct = AddNamespace(api)->addStruct("MyStruct", "1.proto");
    auto apiStruct = api->addStruct("MyStruct", "1.proto");
    apiStruct->addStructField("field1", 1, namespaceStruct->dname());
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Not_Accessible_Type));
}

TEST(ProjectCheckTest, Invalid_Name_Format_Style_Error_If_Struct_Name_Is_Not_CamelCase)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    api->addStruct("myStruct", "1.proto");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(StyleErrc::Invalid_Name_Format));
}

TEST(ProjectCheckTest, Invalid_Name_Format_Style_Error_If_Enum_Name_Is_Not_CamelCase)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    api->addEnum("MY_ENUM", "1.proto");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(StyleErrc::Invalid_Name_Format));
}

TEST(ProjectCheckTest, Invalid_Name_Format_Style_Error_If_Struct_Field_Name_Is_Not_Lowercase_With_Underscores)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    auto structure = api->addStruct("MyStruct", "1.proto");
    structure->addScalarField("MyField", 1, FieldTypeId::Int32);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(StyleErrc::Invalid_Name_Format));
}

TEST(ProjectCheckTest, Invalid_Name_Format_Style_Error_If_Enum_Constant_Name_Is_Not_Uppercase_With_Underscores)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    auto enumeration = api->addEnum("MyEnum", "1.proto");
    enumeration->addConstant("TESt_CONSTANT", 1001);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(StyleErrc::Invalid_Name_Format));
}

TEST(ProjectCheckTest, Undocumented_Entity_Doc_Error_If_Struct_Is_Not_Documented)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    api->addStruct("MyStruct", "1.proto");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(DocErrc::Undocumented_Entity));
}

TEST(ProjectCheckTest, Undocumented_Entity_Doc_Error_If_Enum_Is_Not_Documented)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    api->addEnum("MyEnum", "1.proto");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(DocErrc::Undocumented_Entity));
}

TEST(ProjectCheckTest, Undocumented_Entity_Doc_Error_If_Struct_Field_Is_Not_Documented)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    auto structure = api->addStruct("MyStruct", "1.proto", StructFlags::None, EntityDocs("My struct."));
    structure->addScalarField("field1", 1, FieldTypeId::Int32);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(DocErrc::Undocumented_Entity));
}

TEST(ProjectCheckTest, Undocumented_Entity_Doc_Error_If_Enum_Constant_Is_Not_Documented)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    auto enumeration = api->addEnum("MyEnum", "1.proto", EntityDocs("My enumeration."));
    enumeration->addConstant("CONSTANT_1", 1);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(DocErrc::Undocumented_Entity));
}

TEST(ProjectCheckTest, Unknown_Doc_Command_Doc_Error_If_Struct_Documentation_Command_Is_Unrecognized)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    api->addStruct("MyStruct", "1.proto", StructFlags::None, {{"My structure."}, {{"cmd1", {"value1"}}}});
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(DocErrc::Unknown_Doc_Command));
}

TEST(ProjectCheckTest, Unknown_Doc_Command_Doc_Error_If_Enum_Documentation_Command_Is_Unrecognized)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    api->addEnum("MyEnum", "1.proto", {{"My enumeration."}, {{"cmd1", {"value1"}}}});
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(DocErrc::Unknown_Doc_Command));
}

TEST(ProjectCheckTest, Unknown_Doc_Command_Doc_Error_If_Struct_Field_Documentation_Command_Is_Unrecognized)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    auto structure = api->addStruct("MyStruct", "1.proto", StructFlags::None, EntityDocs("My structure."));
    structure->addScalarField(
        "field1", 1, FieldTypeId::Int32, FieldFlags::None, "", "", {{"Field 1."}, {{"cmd1", {"value1"}}}});
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(DocErrc::Unknown_Doc_Command));
}

TEST(ProjectCheckTest, Unknown_Doc_Command_Doc_Error_If_Enum_Constant_Documentation_Command_Is_Unrecognized)
{
    Project project;
    auto api = project.addApi();
    InitApi(api);

    auto enumeration = api->addEnum("MyEnum", "1.proto", EntityDocs("My enumeration."));
    enumeration->addConstant("CONSTANT_1", 1001, {{"Constant 1."}, {{"cmd1", {"value1"}}}});
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(DocErrc::Unknown_Doc_Command));
}

TEST(ProjectCheckTest, Default_Severity_Of_Errors_Is_SpecErrc_DocErrc_SpecWarn_StyleErrc)
{
    {
        Project project;
        auto api = project.addApi();
        InitApi(api);

        // non-conformat name (should be lowercase)
        auto ns = api->addNamespace("Namespace");

        // unexpected file (should be 'namespace.proto'), undocumented entity
        auto desc = ns->addStruct(GetPredefinedStructName(StructTypeId::Namespace_Desc), "1.proto", StructFlags::None);

        // unexpected nested type
        desc->addStruct("MyStruct", StructFlags::None, EntityDocs("My structure."));

        ErrorCollector ecol = project.check();

        ASSERT_EQ(ecol.errors().size(), 4);
        EXPECT_TRUE(ecol.find(SpecErrc::Missing_Descriptor));
        EXPECT_TRUE(ecol.find(DocErrc::Undocumented_Entity));
        EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
        EXPECT_TRUE(ecol.find(StyleErrc::Invalid_Name_Format));
        EXPECT_EQ(ecol.majorError()->code.category(), spec_error_category());
    }

    {
        Project project;
        auto api = project.addApi();
        InitApi(api);

        // non-conformat name (should be lowercase)
        auto ns = api->addNamespace("Namespace");

        // undocumented entity
        auto desc = ns->addStruct(
            GetPredefinedStructName(StructTypeId::Namespace_Desc), Namespace_Desc_File, StructFlags::None);

        // unexpected nested type
        desc->addStruct("MyStruct", StructFlags::None, EntityDocs("My structure."));

        ErrorCollector ecol = project.check();

        ASSERT_EQ(ecol.errors().size(), 3);
        EXPECT_TRUE(ecol.find(DocErrc::Undocumented_Entity));
        EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
        EXPECT_TRUE(ecol.find(StyleErrc::Invalid_Name_Format));
        EXPECT_EQ(ecol.majorError()->code.category(), doc_error_category());
    }

    {
        Project project;
        auto api = project.addApi();
        InitApi(api);

        // non-conformat name (should be lowercase)
        auto ns = api->addNamespace("Namespace");

        auto desc = ns->addStruct(GetPredefinedStructName(StructTypeId::Namespace_Desc),
                                  Namespace_Desc_File,
                                  StructFlags::None,
                                  EntityDocs("Namespace."));

        // unexpected nested type
        desc->addStruct("MyStruct", StructFlags::None, EntityDocs("My structure."));

        ErrorCollector ecol = project.check();

        ASSERT_EQ(ecol.errors().size(), 2);
        EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
        EXPECT_TRUE(ecol.find(StyleErrc::Invalid_Name_Format));
        EXPECT_EQ(ecol.majorError()->code.category(), spec_warn_category());
    }
}
}} // namespace busrpc::test
