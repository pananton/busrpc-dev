#include "entities/project.h"
#include "utils/common.h"
#include "utils/project_utils.h"

#include <gtest/gtest.h>

namespace busrpc { namespace test {

void InitProject(Project* project)
{
    InitMinimalProject(project);
    AddStructsAndEnums(project);

    AddMethod(AddClass(AddNamespace(AddApi(project))));
    AddImplementation(project);
}

class ProjectCheckTest: public ::testing::Test {
protected:
    void SetUp() override
    {
        InitMinimalProject(&project_);
        api_ = project_.addApi();
        implementation_ = project_.addImplementation();
    }

protected:
    Project project_;
    Api* api_ = nullptr;
    Implementation* implementation_ = nullptr;
};

TEST_F(ProjectCheckTest, Spec_Error_Category_Name_Is_Not_Empty)
{
    EXPECT_TRUE(spec_error_category().name());
    EXPECT_NE(spec_error_category().name()[0], 0);
}

TEST_F(ProjectCheckTest, Spec_Error_Codes_Have_Non_Empty_Descriptions)
{
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(SpecErrc::Invalid_Entity)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(SpecErrc::Multiple_Definitions)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(SpecErrc::Unexpected_Package)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(SpecErrc::Missing_Builtin)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(SpecErrc::Nonconforming_Builtin)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(SpecErrc::No_Descriptor)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(SpecErrc::Not_Static_Method)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(SpecErrc::Not_Encodable_Type)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(SpecErrc::Not_Accessible_Type)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(SpecErrc::Unknown_Type)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(SpecErrc::Unexpected_Type)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(SpecErrc::Empty_Enum)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(SpecErrc::No_Zero_Value)).empty());
    EXPECT_FALSE(spec_error_category().message(static_cast<int>(SpecErrc::Unknown_Method)).empty());
}

TEST_F(ProjectCheckTest, Unknown_Spec_Error_Code_Has_Non_Empty_Description)
{
    EXPECT_FALSE(spec_error_category().message(0).empty());
}

TEST_F(ProjectCheckTest, Spec_Warn_Category_Name_Is_Not_Empty)
{
    EXPECT_TRUE(spec_warn_category().name());
    EXPECT_NE(spec_warn_category().name()[0], 0);
}

TEST_F(ProjectCheckTest, Spec_Warn_Codes_Have_Non_Empty_Descriptions)
{
    EXPECT_FALSE(spec_warn_category().message(static_cast<int>(SpecWarn::Unexpected_Nested_Entity)).empty());
}

TEST_F(ProjectCheckTest, Unknown_Spec_Warn_Code_Has_Non_Empty_Description)
{
    EXPECT_FALSE(spec_warn_category().message(0).empty());
}

TEST_F(ProjectCheckTest, Doc_Warn_Category_Name_Is_Not_Empty)
{
    EXPECT_TRUE(doc_warn_category().name());
    EXPECT_NE(doc_warn_category().name()[0], 0);
}

TEST_F(ProjectCheckTest, Doc_Warn_Codes_Have_Non_Empty_Descriptions)
{
    EXPECT_FALSE(doc_warn_category().message(static_cast<int>(DocWarn::Undocumented_Entity)).empty());
    EXPECT_FALSE(doc_warn_category().message(static_cast<int>(DocWarn::Unknown_Doc_Command)).empty());
}

TEST_F(ProjectCheckTest, Unknown_Doc_Warn_Code_Has_Non_Empty_Description)
{
    EXPECT_FALSE(doc_warn_category().message(0).empty());
}

TEST_F(ProjectCheckTest, Style_Warn_Category_Name_Is_Not_Empty)
{
    EXPECT_TRUE(style_warn_category().name());
    EXPECT_NE(style_warn_category().name()[0], 0);
}

TEST_F(ProjectCheckTest, Style_Warn_Codes_Have_Non_Empty_Descriptions)
{
    EXPECT_FALSE(style_warn_category().message(static_cast<int>(StyleWarn::Invalid_Name_Format)).empty());
}

TEST_F(ProjectCheckTest, Unknown_Style_Warn_Error_Code_Has_Non_Empty_Description)
{
    EXPECT_FALSE(style_warn_category().message(0).empty());
}

TEST_F(ProjectCheckTest, Success_For_Conforming_Project)
{
    Project project;
    InitProject(&project);
    ErrorCollector ecol = project.check();

    EXPECT_EQ(ecol.errors().size(), 0);
}

TEST_F(ProjectCheckTest, Missing_Builtin_Spec_Error_If_Errc_Is_Not_Added)
{
    Project project;
    AddException(&project);
    AddCallMessage(&project);
    AddResultMessage(&project);

    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Builtin));
}

TEST_F(ProjectCheckTest, Missing_Builtin_Spec_Error_If_Errc_Is_Defined_In_Unexpected_File)
{
    Project project;
    AddException(&project);
    AddCallMessage(&project);
    AddResultMessage(&project);

    auto enumeration = project.addEnum(Errc_Enum_Name, "1.proto");
    enumeration->addConstant("CONSTANT_0", 0, EntityDocs("Constant."));
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Builtin));
}

TEST_F(ProjectCheckTest, Missing_Builtin_Spec_Error_If_Exception_Is_Not_Added)
{
    Project project;
    AddErrc(&project);
    AddCallMessage(&project);
    AddResultMessage(&project);

    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Builtin));
}

TEST_F(ProjectCheckTest, Missing_Builtin_Spec_Error_If_Exception_Is_Defined_In_Unexpected_File)
{
    Project project;
    AddErrc(&project);
    AddCallMessage(&project);
    AddResultMessage(&project);

    auto exception = project.addStruct(GetPredefinedStructName(StructTypeId::Exception), "1.proto");
    exception->addEnumField(Exception_Code_Field_Name, 1, JoinStrings(project.dname(), ".Errc"));
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Exception_Does_Not_Have_Code_Field)
{
    Project project;
    AddErrc(&project);
    AddCallMessage(&project);
    AddResultMessage(&project);

    project.addStruct("Exception", Busrpc_Builtin_File, StructFlags::None);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Exception_Code_Field_Is_Not_Errc)
{
    Project project;
    AddErrc(&project);
    AddCallMessage(&project);
    AddResultMessage(&project);

    auto exception =
        project.addStruct(GetPredefinedStructName(StructTypeId::Exception), Busrpc_Builtin_File, StructFlags::None);
    exception->addScalarField(Exception_Code_Field_Name, 1, FieldTypeId::Int32);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Exception_Code_Field_Is_Optional)
{
    Project project;
    AddErrc(&project);
    AddCallMessage(&project);
    AddResultMessage(&project);

    auto exception = project.addStruct(GetPredefinedStructName(StructTypeId::Exception), Busrpc_Builtin_File);
    exception->addEnumField(Exception_Code_Field_Name, 1, JoinStrings(project.dname(), ".Errc"), FieldFlags::Optional);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Exception_Code_Field_Is_Repeated)
{
    Project project;
    AddErrc(&project);
    AddCallMessage(&project);
    AddResultMessage(&project);

    auto exception = project.addStruct(GetPredefinedStructName(StructTypeId::Exception), Busrpc_Builtin_File);
    exception->addEnumField(Exception_Code_Field_Name, 1, JoinStrings(project.dname(), ".Errc"), FieldFlags::Repeated);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Missing_Builtin_Spec_Error_If_Call_Message_Is_Not_Added)
{
    Project project;
    AddErrc(&project);
    AddException(&project);
    AddResultMessage(&project);

    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Builtin));
}

TEST_F(ProjectCheckTest, Missing_Builtin_Spec_Error_If_Call_Message_Is_Defined_In_Unexpected_File)
{
    Project project;
    AddErrc(&project);
    AddException(&project);
    AddResultMessage(&project);

    auto call = project.addStruct(GetPredefinedStructName(StructTypeId::Call_Message), "1.proto");
    call->addScalarField(Call_Message_Object_Id_Field_Name, 5, FieldTypeId::Bytes, FieldFlags::Optional);
    call->addScalarField(Call_Message_Params_Field_Name, 6, FieldTypeId::Bytes, FieldFlags::Optional);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Call_Message_Does_Not_Have_Object_Id_Field)
{
    Project project;
    AddErrc(&project);
    AddException(&project);
    AddResultMessage(&project);

    auto call = project.addStruct(GetPredefinedStructName(StructTypeId::Call_Message), Busrpc_Builtin_File);
    call->addScalarField(Call_Message_Params_Field_Name, 6, FieldTypeId::Bytes, FieldFlags::Optional);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Call_Message_Does_Not_Have_Params_Field)
{
    Project project;
    AddErrc(&project);
    AddException(&project);
    AddResultMessage(&project);

    auto call = project.addStruct(GetPredefinedStructName(StructTypeId::Call_Message), Busrpc_Builtin_File);
    call->addScalarField(Call_Message_Object_Id_Field_Name, 5, FieldTypeId::Bytes, FieldFlags::Optional);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Call_Message_Object_Id_Field_Is_Not_Bytes)
{
    Project project;
    AddErrc(&project);
    AddException(&project);
    AddResultMessage(&project);

    auto call = project.addStruct(GetPredefinedStructName(StructTypeId::Call_Message), Busrpc_Builtin_File);
    call->addScalarField(Call_Message_Object_Id_Field_Name, 5, FieldTypeId::Int32, FieldFlags::Optional);
    call->addScalarField(Call_Message_Params_Field_Name, 6, FieldTypeId::Bytes, FieldFlags::Optional);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Call_Message_Params_Field_Is_Not_Bytes)
{
    Project project;
    AddErrc(&project);
    AddException(&project);
    AddResultMessage(&project);

    auto call = project.addStruct(GetPredefinedStructName(StructTypeId::Call_Message), Busrpc_Builtin_File);
    call->addScalarField(Call_Message_Object_Id_Field_Name, 5, FieldTypeId::Bytes, FieldFlags::Optional);
    call->addScalarField(Call_Message_Params_Field_Name, 6, FieldTypeId::Int32, FieldFlags::Optional);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Call_Message_Object_Id_Field_Is_Not_Optional)
{
    Project project;
    AddErrc(&project);
    AddException(&project);
    AddResultMessage(&project);

    auto call = project.addStruct(GetPredefinedStructName(StructTypeId::Call_Message), Busrpc_Builtin_File);
    call->addScalarField(Call_Message_Object_Id_Field_Name, 5, FieldTypeId::Bytes);
    call->addScalarField(Call_Message_Params_Field_Name, 6, FieldTypeId::Bytes, FieldFlags::Optional);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Call_Message_Params_Field_Is_Not_Optional)
{
    Project project;
    AddErrc(&project);
    AddException(&project);
    AddResultMessage(&project);

    auto call = project.addStruct(GetPredefinedStructName(StructTypeId::Call_Message), Busrpc_Builtin_File);
    call->addScalarField(Call_Message_Object_Id_Field_Name, 5, FieldTypeId::Bytes, FieldFlags::Optional);
    call->addScalarField(Call_Message_Params_Field_Name, 6, FieldTypeId::Bytes);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Call_Message_Has_Unknown_Fields)
{
    Project project;
    AddErrc(&project);
    AddException(&project);
    AddResultMessage(&project);

    auto call = AddCallMessage(&project);
    call->addScalarField("unknown_field", 7, FieldTypeId::Bytes, FieldFlags::Optional);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Missing_Builtin_Spec_Error_If_Result_Message_Is_Not_Added)
{
    Project project;
    AddErrc(&project);
    AddException(&project);
    AddCallMessage(&project);

    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Builtin));
}

TEST_F(ProjectCheckTest, Missing_Builtin_Spec_Error_If_Result_Message_Is_Defined_In_Unexpected_File)
{
    Project project;
    AddErrc(&project);
    AddException(&project);
    AddCallMessage(&project);

    auto result = project.addStruct(GetPredefinedStructName(StructTypeId::Result_Message), "1.proto");
    result->addScalarField(Result_Message_Retval_Field_Name, 5, FieldTypeId::Bytes, FieldFlags::None, "Result");
    result->addStructField(
        Result_Message_Exception_Field_Name, 6, JoinStrings(project.dname(), ".Exception"), FieldFlags::None, "Result");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Missing_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Result_Message_Does_Not_Have_Retval_Field)
{
    Project project;
    AddErrc(&project);
    AddException(&project);
    AddCallMessage(&project);

    auto result = project.addStruct(GetPredefinedStructName(StructTypeId::Result_Message), Busrpc_Builtin_File);
    result->addStructField(
        Result_Message_Exception_Field_Name, 6, JoinStrings(project.dname(), ".Exception"), FieldFlags::None, "Result");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Result_Message_Does_Not_Have_Exception_Field)
{
    Project project;
    AddErrc(&project);
    AddException(&project);
    AddCallMessage(&project);

    auto result = project.addStruct(GetPredefinedStructName(StructTypeId::Result_Message), Busrpc_Builtin_File);
    result->addScalarField(Result_Message_Retval_Field_Name, 5, FieldTypeId::Bytes, FieldFlags::None, "Result");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Result_Message_Retval_Field_Is_Not_Bytes)
{
    Project project;
    AddErrc(&project);
    AddException(&project);
    AddCallMessage(&project);

    auto result = project.addStruct(GetPredefinedStructName(StructTypeId::Result_Message), Busrpc_Builtin_File);
    result->addScalarField(Result_Message_Retval_Field_Name, 5, FieldTypeId::Int32, FieldFlags::None, "Result");
    result->addStructField(
        Result_Message_Exception_Field_Name, 6, JoinStrings(project.dname(), ".Exception"), FieldFlags::None, "Result");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Result_Message_Exception_Field_Is_Not_Exception)
{
    Project project;
    AddErrc(&project);
    AddException(&project);
    AddCallMessage(&project);

    auto result = project.addStruct(GetPredefinedStructName(StructTypeId::Result_Message), Busrpc_Builtin_File);
    result->addScalarField(Result_Message_Retval_Field_Name, 5, FieldTypeId::Bytes, FieldFlags::None, "Result");
    result->addScalarField(Result_Message_Exception_Field_Name, 6, FieldTypeId::Int32, FieldFlags::None, "Result");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Result_Message_Retval_Field_Is_Not_Part_Of_Oneof)
{
    Project project;
    AddErrc(&project);
    AddException(&project);
    AddCallMessage(&project);

    auto result = project.addStruct(GetPredefinedStructName(StructTypeId::Result_Message), Busrpc_Builtin_File);
    result->addScalarField(Result_Message_Retval_Field_Name, 5, FieldTypeId::Bytes, FieldFlags::None);
    result->addStructField(
        Result_Message_Exception_Field_Name, 6, JoinStrings(project.dname(), ".Exception"), FieldFlags::None, "Result");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Result_Message_Exception_Field_Is_Not_Part_Of_Oneof)
{
    Project project;
    AddErrc(&project);
    AddException(&project);
    AddCallMessage(&project);

    auto result = project.addStruct(GetPredefinedStructName(StructTypeId::Result_Message), Busrpc_Builtin_File);
    result->addScalarField(Result_Message_Retval_Field_Name, 5, FieldTypeId::Bytes, FieldFlags::None, "Result");
    result->addStructField(
        Result_Message_Exception_Field_Name, 6, JoinStrings(project.dname(), ".Exception"), FieldFlags::None);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest,
       Nonconforming_Builtin_Spec_Error_If_Result_Message_Retval_And_Exception_Fields_Does_Not_Belong_To_Same_Oneof)
{
    Project project;
    AddErrc(&project);
    AddException(&project);
    AddCallMessage(&project);

    auto result = project.addStruct(GetPredefinedStructName(StructTypeId::Result_Message), Busrpc_Builtin_File);
    result->addScalarField(Result_Message_Retval_Field_Name, 5, FieldTypeId::Bytes, FieldFlags::None, "Result");
    result->addStructField(Result_Message_Exception_Field_Name,
                           6,
                           JoinStrings(project.dname(), ".Exception"),
                           FieldFlags::None,
                           "Result1");
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, Nonconforming_Builtin_Spec_Error_If_Result_Message_Has_Unknown_Fields)
{
    Project project;
    AddErrc(&project);
    AddException(&project);
    AddCallMessage(&project);

    auto result = AddResultMessage(&project);
    result->addScalarField("unknown_field", 7, FieldTypeId::Int32);
    auto ecol = project.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Nonconforming_Builtin));
}

TEST_F(ProjectCheckTest, No_Descriptor_Spec_Error_If_Namespace_Does_Not_Have_Descriptor)
{
    api_->addNamespace("namespace");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::No_Descriptor));
}

TEST_F(ProjectCheckTest, No_Descriptor_Spec_Error_If_Namespace_Descriptor_Is_Defined_In_Unexpected_File)
{
    auto ns = api_->addNamespace("namespace");
    ns->addStruct(GetPredefinedStructName(StructTypeId::Namespace_Desc), "1.proto");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::No_Descriptor));
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
    auto enumeration = AddNamespaceDesc(ns)->addEnum("NestedEnum", EntityDocs("Nested enum."));
    enumeration->addConstant("CONSTANT_0", 0, EntityDocs("Constant."));
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

TEST_F(ProjectCheckTest, Invalid_Name_Format_Style_Warn_If_Namespace_Name_Is_Not_Lowercase_With_Underscores)
{
    auto ns = api_->addNamespace("Namespace");
    AddNamespaceDesc(ns);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(StyleWarn::Invalid_Name_Format));
}

TEST_F(ProjectCheckTest, No_Descriptor_Spec_Error_If_Class_Does_Not_Have_Descriptor)
{
    AddNamespace(api_)->addClass("class");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::No_Descriptor));
}

TEST_F(ProjectCheckTest, No_Descriptor_Spec_Error_If_Class_Descriptor_Is_Defined_In_Unexpected_File)
{
    auto cls = AddNamespace(api_)->addClass("class");
    cls->addStruct(GetPredefinedStructName(StructTypeId::Class_Desc), "1.proto");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::No_Descriptor));
}

TEST_F(ProjectCheckTest, Not_Encodable_Type_Spec_Error_If_Class_Object_Id_Is_Not_Encodable)
{
    auto cls = AddNamespace(api_)->addClass("class");
    auto desc = cls->addStruct(GetPredefinedStructName(StructTypeId::Class_Desc), "1.proto");
    auto oid = desc->addStruct(GetPredefinedStructName(StructTypeId::Class_Object_Id));
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
    auto enumeration = AddClassDesc(cls)->addEnum("NestedEnum", EntityDocs("Nested enum."));
    enumeration->addConstant("CONSTANT_0", 0, EntityDocs("Constant."));
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

TEST_F(ProjectCheckTest, Invalid_Name_Format_Style_Warn_If_Class_Name_Is_Not_Lowercase_With_Underscores)
{
    auto cls = AddNamespace(api_)->addClass("clasS");
    AddClassDesc(cls);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(StyleWarn::Invalid_Name_Format));
}

TEST_F(ProjectCheckTest, No_Descriptor_Spec_Error_If_Method_Does_Not_Have_Descriptor)
{
    AddClass(AddNamespace(api_))->addMethod("method");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::No_Descriptor));
}

TEST_F(ProjectCheckTest, No_Descriptor_Spec_Error_If_Method_Descriptor_Is_Defined_In_Unexpected_File)
{
    auto method = AddClass(AddNamespace(api_))->addMethod("method");
    method->addStruct(GetPredefinedStructName(StructTypeId::Method_Desc), "1.proto");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::No_Descriptor));
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
    auto enumeration = AddMethodDesc(method)->addEnum("NestedEnum", EntityDocs("Nested enum."));
    enumeration->addConstant("CONSTANT_0", 0, EntityDocs("Constant."));
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

TEST_F(ProjectCheckTest, Invalid_Name_Format_Style_Warn_If_Method_Name_Is_Not_Lowercase_With_Underscores)
{
    auto method = AddClass(AddNamespace(api_))->addMethod("meThod");
    AddMethodDesc(method);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(StyleWarn::Invalid_Name_Format));
}

TEST_F(ProjectCheckTest, No_Descriptor_Spec_Error_If_Service_Does_Not_Have_Descriptor)
{
    implementation_->addService("service");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::No_Descriptor));
}

TEST_F(ProjectCheckTest, No_Descriptor_Spec_Error_If_Service_Descriptor_Is_Defined_In_Unexpected_File)
{
    auto service = implementation_->addService("service");
    service->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), "1.proto");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::No_Descriptor));
}

TEST_F(ProjectCheckTest,
       Unknown_Method_Spec_Error_If_Service_Implements_Contains_Field_Whose_Type_Is_Not_Existing_Method_Descriptor)
{
    auto service = implementation_->addService("service");
    auto desc = service->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), Service_Desc_File);
    auto implements = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Implements));
    implements->addScalarField("field1", 1, FieldTypeId::Int32);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Unknown_Method));
}

TEST_F(ProjectCheckTest,
       Unknown_Method_Spec_Error_If_Service_Invokes_Contains_Field_Whose_Type_Is_Not_Existing_Method_Descriptor)
{
    auto service = implementation_->addService("service");
    auto desc = service->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), Service_Desc_File);
    auto invokes = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Invokes));
    invokes->addScalarField("field1", 1, FieldTypeId::Int32);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Unknown_Method));
}

TEST_F(ProjectCheckTest, Multiple_Definitions_Spec_Error_If_Service_Implements_References_Same_Method_More_Than_Once)
{
    auto method = AddMethod(AddClass(AddNamespace(api_)));
    auto service = implementation_->addService("service");
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
    auto service = implementation_->addService("service");
    auto desc = service->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), Service_Desc_File);

    auto invokes = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Invokes));
    invokes->addStructField("field1", 1, method->descriptor()->dname());
    invokes->addStructField("field2", 2, method->descriptor()->dname());

    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Multiple_Definitions));
}

TEST_F(ProjectCheckTest, Unexpected_Nested_Entity_Spec_Warn_If_Service_Descriptor_Has_Unexpected_Nested_Struct)
{
    auto service = implementation_->addService("service");
    AddServiceDesc(service)->addStruct("UnexpectedStruct", StructFlags::None, EntityDocs("Unexpected nested struct."));
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST_F(ProjectCheckTest, Unexpected_Nested_Entity_Spec_Warn_If_Service_Descriptor_Has_Unexpected_Nested_Enum)
{
    auto service = implementation_->addService("service");
    auto enumeration = AddServiceDesc(service)->addEnum("UnexpectedEnum", EntityDocs("Unexpected nested enum."));
    enumeration->addConstant("CONSTANT_0", 0, EntityDocs("Constant."));
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST_F(ProjectCheckTest, Unexpected_Nested_Entity_Spec_Warn_If_Service_Descriptor_Has_Unexpected_Fields)
{
    auto service = implementation_->addService("service");
    AddServiceDesc(service)->addScalarField(
        "field", 1, FieldTypeId::Int32, FieldFlags::None, "", "", EntityDocs("Field."));
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecWarn::Unexpected_Nested_Entity));
}

TEST_F(ProjectCheckTest, Invalid_Name_Format_Style_Warn_If_Service_Name_Is_Not_Lowercase_With_Underscores)
{
    auto service = implementation_->addService("Service");
    AddServiceDesc(service);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(StyleWarn::Invalid_Name_Format));
}

TEST_F(ProjectCheckTest, Empty_Enum_Spec_Error_If_Enum_Does_Not_Have_Constants)
{
    api_->addEnum("MyEnum", "1.proto");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Empty_Enum));
}

TEST_F(ProjectCheckTest, No_Zero_Value_Spec_Error_If_Enum_Does_Not_Have_Constant_With_Zero_Value)
{
    auto enumeration = api_->addEnum("MyEnum", "1.proto");
    enumeration->addConstant("MY_ENUM_1", 1);
    enumeration->addConstant("MY_ENUM_2", 2);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::No_Zero_Value));
}

TEST_F(ProjectCheckTest, Unknown_Type_Spec_Error_If_Struct_Type_Of_The_Field_Is_Unknown)
{
    auto structure = project_.addStruct("MyStruct", "1.proto");
    structure->addStructField("field1", 1, "UnknownStruct");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Unknown_Type));
}

TEST_F(ProjectCheckTest, Unknown_Type_Spec_Error_If_Enum_Type_Of_The_Field_Is_Unknown)
{
    auto structure = project_.addStruct("MyStruct", "1.proto");
    structure->addEnumField("field1", 1, "UnknownEnum");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Unknown_Type));
}

TEST_F(ProjectCheckTest, Unknown_Type_Spec_Error_If_Struct_Value_Type_Of_The_Map_Field_Is_Unknown)
{
    auto structure = project_.addStruct("MyStruct", "1.proto");
    structure->addMapField("field1", 1, FieldTypeId::Int32, FieldTypeId::Message, "UnknownStruct");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Unknown_Type));
}

TEST_F(ProjectCheckTest, Unknown_Type_Spec_Error_If_Enum_Value_Type_Of_The_Map_Field_Is_Unknown)
{
    auto structure = project_.addStruct("MyStruct", "1.proto");
    structure->addMapField("field1", 1, FieldTypeId::Int32, FieldTypeId::Enum, "UnknownEnum");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Unknown_Type));
}

TEST_F(ProjectCheckTest, Unexpected_Type_Spec_Error_If_Struct_Type_Of_The_Field_Is_Not_Struct_Entity)
{
    auto enumeration = project_.addEnum("MyEnum", "1.proto");
    enumeration->addConstant("CONSTANT_0", 0, EntityDocs("Constant."));
    auto structure = project_.addStruct("MyStruct", "1.proto");
    structure->addStructField("field1", 1, enumeration->dname());
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Unexpected_Type));
}

TEST_F(ProjectCheckTest, Unexpected_Type_Spec_Error_If_Enum_Type_Of_The_Field_Is_Not_Enum_Entity)
{
    auto structure1 = project_.addStruct("MyStruct1", "1.proto");
    auto structure2 = project_.addStruct("MyStruct2", "1.proto");
    structure2->addEnumField("field1", 1, structure1->dname());
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Unexpected_Type));
}

TEST_F(ProjectCheckTest, Unexpected_Type_Spec_Error_If_Struct_Value_Type_Of_The_Map_Field_Is_Not_Struct_Entity)
{
    auto enumeration = project_.addEnum("MyEnum", "1.proto");
    enumeration->addConstant("CONSTANT_0", 0, EntityDocs("Constant."));
    auto structure = project_.addStruct("MyStruct", "1.proto");
    structure->addMapField("field1", 1, FieldTypeId::Int32, FieldTypeId::Message, enumeration->dname());
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Unexpected_Type));
}

TEST_F(ProjectCheckTest, Unexpected_Type_Spec_Error_If_Enum_Value_Type_Of_The_Map_Field_Is_Not_Enum_Entity)
{
    auto structure1 = project_.addStruct("MyStruct1", "1.proto");
    auto structure2 = project_.addStruct("MyStruct2", "1.proto");
    structure2->addMapField("field1", 1, FieldTypeId::Int32, FieldTypeId::Enum, structure1->dname());
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Unexpected_Type));
}

TEST_F(ProjectCheckTest, Not_Encodable_Type_Spec_Error_If_Non_Encodable_Structure_Is_Marked_As_Hashed)
{
    auto nonEncodable = project_.addStruct("MyStruct", "1.proto", StructFlags::Hashed);
    nonEncodable->addScalarField("field1", 1, FieldTypeId::Int32, FieldFlags::Repeated);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Not_Encodable_Type));
}

TEST_F(ProjectCheckTest, Not_Encodable_Type_Spec_Error_If_Field_Of_Non_Encodable_Type_Is_Marked_As_Observable)
{
    auto structure = project_.addStruct("MyStruct", "1.proto");
    structure->addScalarField("field1", 1, FieldTypeId::Int32, FieldFlags::Observable, "oneofName");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Not_Encodable_Type));
}

TEST_F(ProjectCheckTest, Not_Encodable_Type_Spec_Error_If_Field_Of_Non_Encodable_Type_Is_Marked_As_Hashed)
{
    auto structure = project_.addStruct("MyStruct", "1.proto");
    structure->addScalarField("field1", 1, FieldTypeId::Int32, FieldFlags::Hashed, "oneofName");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Not_Encodable_Type));
}

TEST_F(ProjectCheckTest, Not_Encodable_Type_Spec_Error_If_Field_Of_Non_Encodable_Structure_Type_Is_Marked_As_Observable)
{
    auto structure1 = project_.addStruct("MyStruct1", "1.proto");
    structure1->addScalarField("field1", 1, FieldTypeId::Double);

    auto structure2 = project_.addStruct("MyStruct2", "2.proto");
    structure2->addStructField("field1", 1, structure1->dname(), FieldFlags::Observable);

    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Not_Encodable_Type));
}

TEST_F(ProjectCheckTest, Not_Encodable_Type_Spec_Error_If_Field_Of_Non_Encodable_Structure_Type_Is_Marked_As_Hashed)
{
    auto structure1 = project_.addStruct("MyStruct1", "1.proto");
    structure1->addScalarField("field1", 1, FieldTypeId::Double);

    auto structure2 = project_.addStruct("MyStruct2", "2.proto");
    structure2->addStructField("field1", 1, structure1->dname(), FieldFlags::Hashed);

    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Not_Encodable_Type));
}

TEST_F(ProjectCheckTest, Not_Encodable_Type_Spec_Error_If_Field_Of_Google_Type_Is_Marked_As_Observable)
{
    auto structure = project_.addStruct("MyStruct", "1.proto");
    structure->addStructField("field1", 1, "google.protobuf.Any", FieldFlags::Observable);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Not_Encodable_Type));
}

TEST_F(ProjectCheckTest, Not_Encodable_Type_Spec_Error_If_Field_Google_Type_Is_Marked_As_Hashed)
{
    auto structure = project_.addStruct("MyStruct", "1.proto");
    structure->addStructField("field1", 1, "google.protobuf.Any", FieldFlags::Hashed);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Not_Encodable_Type));
}

TEST_F(ProjectCheckTest, Not_Accessible_Type_Spec_Error_If_Referenced_Type_Outside_The_Current_Scope)
{
    auto apiStruct = api_->addStruct("MyStruct", "1.proto");
    auto projectStruct = project_.addStruct("MyStruct", "1.proto");
    projectStruct->addStructField("field1", 1, apiStruct->dname());
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(SpecErrc::Not_Accessible_Type));
}

TEST_F(ProjectCheckTest, Invalid_Name_Format_Style_Warn_If_Struct_Name_Is_Not_CamelCase)
{
    project_.addStruct("myStruct", "1.proto");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(StyleWarn::Invalid_Name_Format));
}

TEST_F(ProjectCheckTest, Invalid_Name_Format_Style_Warn_If_Enum_Name_Is_Not_CamelCase)
{
    auto enumeration = project_.addEnum("MY_ENUM", "1.proto");
    enumeration->addConstant("CONSTANT_0", 0, EntityDocs("Constant."));
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(StyleWarn::Invalid_Name_Format));
}

TEST_F(ProjectCheckTest, Invalid_Name_Format_Style_Warn_If_Struct_Field_Name_Is_Not_Lowercase_With_Underscores)
{
    auto structure = project_.addStruct("MyStruct", "1.proto");
    structure->addScalarField("MyField", 1, FieldTypeId::Int32);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(StyleWarn::Invalid_Name_Format));
}

TEST_F(ProjectCheckTest, Invalid_Name_Format_Style_Warn_If_Enum_Constant_Name_Is_Not_Uppercase_With_Underscores)
{
    auto enumeration = project_.addEnum("MyEnum", "1.proto");
    enumeration->addConstant("TESt_CONSTANT", 0);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(StyleWarn::Invalid_Name_Format));
}

TEST_F(ProjectCheckTest, Undocumented_Entity_Doc_Warn_If_Struct_Is_Not_Documented)
{
    project_.addStruct("MyStruct", "1.proto");
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(DocWarn::Undocumented_Entity));
}

TEST_F(ProjectCheckTest, Undocumented_Entity_Doc_Warn_If_Enum_Is_Not_Documented)
{
    auto enumeration = project_.addEnum("MyEnum", "1.proto");
    enumeration->addConstant("CONSTANT_0", 0, EntityDocs("Constant."));
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(DocWarn::Undocumented_Entity));
}

TEST_F(ProjectCheckTest, Undocumented_Entity_Doc_Warn_If_Struct_Field_Is_Not_Documented)
{
    auto structure = project_.addStruct("MyStruct", "1.proto", StructFlags::None, EntityDocs("My struct."));
    structure->addScalarField("field1", 1, FieldTypeId::Int32);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(DocWarn::Undocumented_Entity));
}

TEST_F(ProjectCheckTest, Undocumented_Entity_Doc_Warn_If_Enum_Constant_Is_Not_Documented)
{
    auto enumeration = project_.addEnum("MyEnum", "1.proto", EntityDocs("My enumeration."));
    enumeration->addConstant("CONSTANT_0", 0);
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(DocWarn::Undocumented_Entity));
}

TEST_F(ProjectCheckTest, Unknown_Doc_Command_Doc_Warn_If_Struct_Documentation_Command_Is_Unrecognized)
{
    project_.addStruct("MyStruct", "1.proto", StructFlags::None, {{"My structure."}, {{"cmd1", {"value1"}}}});
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(DocWarn::Unknown_Doc_Command));
}

TEST_F(ProjectCheckTest, Unknown_Doc_Command_Doc_Warn_If_Enum_Documentation_Command_Is_Unrecognized)
{
    auto enumeration = project_.addEnum("MyEnum", "1.proto", {{"My enumeration."}, {{"cmd1", {"value1"}}}});
    enumeration->addConstant("CONSTANT_0", 0, EntityDocs("Constant."));
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(DocWarn::Unknown_Doc_Command));
}

TEST_F(ProjectCheckTest, Unknown_Doc_Command_Doc_Warn_If_Struct_Field_Documentation_Command_Is_Unrecognized)
{
    auto structure = project_.addStruct("MyStruct", "1.proto", StructFlags::None, EntityDocs("My structure."));
    structure->addScalarField(
        "field1", 1, FieldTypeId::Int32, FieldFlags::None, "", "", {{"Field 1."}, {{"cmd1", {"value1"}}}});
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(DocWarn::Unknown_Doc_Command));
}

TEST_F(ProjectCheckTest, Unknown_Doc_Command_Doc_Warn_If_Enum_Constant_Documentation_Command_Is_Unrecognized)
{
    auto enumeration = project_.addEnum("MyEnum", "1.proto", EntityDocs("My enumeration."));
    enumeration->addConstant("CONSTANT_0", 0, {{"Constant 1."}, {{"cmd1", {"value1"}}}});
    auto ecol = project_.check();

    EXPECT_TRUE(ecol.find(DocWarn::Unknown_Doc_Command));
}

TEST_F(ProjectCheckTest, Default_Severity_Of_Errors_Is_SpecErrc_SpecWarn_DocWarn_StyleWarn)
{
    {
        Project project;
        InitMinimalProject(&project);
        auto api = project.addApi();

        auto ns = api->addNamespace("Namespace"); // non-conformat name (style warn)
        auto desc = ns->addStruct(                // unexpected file (spec error), undocumented entity (doc warn)
            GetPredefinedStructName(StructTypeId::Namespace_Desc),
            "1.proto");
        desc->addStruct(
            "MyStruct", StructFlags::None, EntityDocs("My structure.")); // unexpected nested type (spec warn)

        ErrorCollector ecol = project.check();

        EXPECT_EQ(ecol.majorError()->code.category(), spec_error_category());
        EXPECT_NE(ecol.majorError()->description.find("namespace="), std::string::npos);
    }

    {
        Project project;
        InitMinimalProject(&project);
        auto api = project.addApi();

        auto ns = api->addNamespace("Namespace"); // non-conformat name (style warn)
        auto desc = ns->addStruct(                // undocumented entity (doc warn)
            GetPredefinedStructName(StructTypeId::Namespace_Desc),
            Namespace_Desc_File,
            StructFlags::None);
        desc->addStruct(
            "MyStruct", StructFlags::None, EntityDocs("My structure.")); // unexpected nested type (spec warn)

        ErrorCollector ecol = project.check();

        EXPECT_EQ(ecol.majorError()->code.category(), spec_warn_category());
    }

    {
        Project project;
        InitMinimalProject(&project);
        auto api = project.addApi();

        auto ns = api->addNamespace("Namespace"); // non-conformat name (style warn)
        ns->addStruct(                            // undocumented entity (doc warn)
            GetPredefinedStructName(StructTypeId::Namespace_Desc),
            Namespace_Desc_File,
            StructFlags::None);

        ErrorCollector ecol = project.check();

        EXPECT_EQ(ecol.majorError()->code.category(), doc_warn_category());
    }

    {
        Project project;
        InitMinimalProject(&project);
        auto api = project.addApi();

        auto ns = api->addNamespace("Namespace"); // non-conformat name (should be lowercase)
        ns->addStruct(GetPredefinedStructName(StructTypeId::Namespace_Desc),
                      Namespace_Desc_File,
                      StructFlags::None,
                      EntityDocs("Namespace."));

        ErrorCollector ecol = project.check();

        EXPECT_EQ(ecol.majorError()->code.category(), style_warn_category());
    }
}

TEST_F(ProjectCheckTest, Empty_Struct_Can_Be_Marked_As_Hashed)
{
    project_.addStruct("MyStruct", "1.proto", StructFlags::Hashed, EntityDocs("Structure."));
    auto ecol = project_.check();

    EXPECT_FALSE(ecol);
}
}} // namespace busrpc::test
