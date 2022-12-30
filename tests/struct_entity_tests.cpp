#include "entities/project.h"
#include "utils/common.h"

#include <gtest/gtest.h>

namespace busrpc { namespace test {

class StructEntityTest: public ::testing::Test {
protected:
    void SetUp() override
    {
        docs_ = EntityDocs({"Brief description.", "Description"}, {{"cmd", {"cmd value"}}});

        auto api = project_.addApi();
        auto ns = api->addNamespace("namespace");
        auto cls = ns->addClass("class");
        auto method = cls->addMethod("method");

        method_ = method;
        structure_ = method->addStruct("Struct", "struct.proto", StructFlags::Hashed, docs_);
    }

protected:
    Project project_;
    EntityDocs docs_;
    const Method* method_ = nullptr;
    Struct* structure_ = nullptr;
};

TEST_F(StructEntityTest, Struct_Entity_Is_Correctly_Initialized_When_Created_By_Parent_Entity_With_Non_Struct_Type)
{
    EXPECT_EQ(structure_->type(), EntityTypeId::Struct);
    EXPECT_EQ(structure_->name(), "Struct");
    EXPECT_EQ(structure_->dir(), std::filesystem::path(Api_Entity_Name) / "namespace" / "class" / "method");
    EXPECT_EQ(structure_->dname(),
              JoinStrings(Project_Entity_Name, ".", Api_Entity_Name, ".namespace.class.method.Struct"));
    EXPECT_EQ(structure_->parent(), method_);
    EXPECT_EQ(static_cast<const Struct*>(structure_)->parent(), method_);
    EXPECT_EQ(structure_->package(), JoinStrings(Project_Entity_Name, ".", Api_Entity_Name, ".namespace.class.method"));
    EXPECT_EQ(structure_->structType(), StructTypeId::General);
    EXPECT_EQ(structure_->file(), structure_->dir() / "struct.proto");
    EXPECT_EQ(structure_->flags(), StructFlags::Hashed);
    EXPECT_TRUE(structure_->isHashed());
    EXPECT_TRUE(structure_->fields().empty());

    EXPECT_EQ(structure_->docs().description(), docs_.description());
    EXPECT_EQ(structure_->docs().brief(), docs_.brief());
    EXPECT_EQ(structure_->docs().commands(), docs_.commands());
}

TEST_F(StructEntityTest, Class_Object_Id_Struct_Has_Default_Description)
{
    Project project;
    auto cls = project.addApi()->addNamespace("namespace")->addClass("class");
    auto desc = cls->addStruct(GetPredefinedStructName(StructTypeId::Class_Desc), Class_Desc_File);
    auto oid = desc->addStruct(GetPredefinedStructName(StructTypeId::Class_Object_Id));

    EXPECT_FALSE(oid->docs().description().empty());
    EXPECT_FALSE(oid->docs().brief().empty());
}

TEST_F(StructEntityTest, Method_Params_Struct_Has_Default_Description)
{
    Project project;
    auto method = project.addApi()->addNamespace("namespace")->addClass("class")->addMethod("method");
    auto desc = method->addStruct(GetPredefinedStructName(StructTypeId::Method_Desc), Method_Desc_File);
    auto params = desc->addStruct(GetPredefinedStructName(StructTypeId::Method_Params));

    EXPECT_FALSE(params->docs().description().empty());
    EXPECT_FALSE(params->docs().brief().empty());
}

TEST_F(StructEntityTest, Method_Retval_Struct_Has_Default_Description)
{
    Project project;
    auto method = project.addApi()->addNamespace("namespace")->addClass("class")->addMethod("method");
    auto desc = method->addStruct(GetPredefinedStructName(StructTypeId::Method_Desc), Method_Desc_File);
    auto retval = desc->addStruct(GetPredefinedStructName(StructTypeId::Method_Retval));

    EXPECT_FALSE(retval->docs().description().empty());
    EXPECT_FALSE(retval->docs().brief().empty());
}

TEST_F(StructEntityTest, Method_Static_Marker_Has_Default_Description)
{
    Project project;
    auto method = project.addApi()->addNamespace("namespace")->addClass("class")->addMethod("method");
    auto desc = method->addStruct(GetPredefinedStructName(StructTypeId::Method_Desc), Method_Desc_File);
    auto staticMarker = desc->addStruct(GetPredefinedStructName(StructTypeId::Method_Static_Marker));

    EXPECT_FALSE(staticMarker->docs().description().empty());
    EXPECT_FALSE(staticMarker->docs().brief().empty());
}

TEST_F(StructEntityTest, Service_Config_Struct_Has_Default_Description)
{
    Project project;
    auto service = project.addImplementation()->addService("service");
    auto desc = service->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), Service_Desc_File);
    auto config = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Config));

    EXPECT_FALSE(config->docs().description().empty());
    EXPECT_FALSE(config->docs().brief().empty());
}

TEST_F(StructEntityTest, Service_Implements_Struct_Has_Default_Description)
{
    Project project;
    auto service = project.addImplementation()->addService("service");
    auto desc = service->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), Service_Desc_File);
    auto implements = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Implements));

    EXPECT_FALSE(implements->docs().description().empty());
    EXPECT_FALSE(implements->docs().brief().empty());
}

TEST_F(StructEntityTest, Service_Invokes_Struct_Has_Default_Description)
{
    Project project;
    auto service = project.addImplementation()->addService("service");
    auto desc = service->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), Service_Desc_File);
    auto invokes = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Invokes));

    EXPECT_FALSE(invokes->docs().description().empty());
    EXPECT_FALSE(invokes->docs().brief().empty());
}

TEST_F(StructEntityTest, Default_Struct_Description_Does_Not_Overwrite_Explicitly_Set_Description)
{
    Project project;
    auto service = project.addImplementation()->addService("service");
    auto desc = service->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), Service_Desc_File);
    auto implements = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Implements),
                                      StructFlags::None,
                                      EntityDocs({"Text."}, {{"cmd", {"value"}}}));

    ASSERT_EQ(implements->docs().description().size(), 1);
    EXPECT_EQ(implements->docs().description()[0], "Text.");
    EXPECT_FALSE(implements->docs().brief().empty());
    EXPECT_EQ(implements->docs().commands().size(), 1);
    ASSERT_NE(implements->docs().commands().find("cmd"), implements->docs().commands().end());
    EXPECT_EQ(implements->docs().commands().find("cmd")->second, std::vector<std::string>{"value"});
}

TEST_F(StructEntityTest, Default_Struct_Description_Is_Merged_With_Explicitly_Set_Doc_Commands)
{
    Project project;
    auto service = project.addImplementation()->addService("service");
    auto desc = service->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), Service_Desc_File);
    auto implements = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Implements),
                                      StructFlags::None,
                                      EntityDocs({}, {{"cmd", {"value"}}}));

    EXPECT_FALSE(implements->docs().description().empty());
    EXPECT_FALSE(implements->docs().brief().empty());
    EXPECT_EQ(implements->docs().commands().size(), 1);
    ASSERT_NE(implements->docs().commands().find("cmd"), implements->docs().commands().end());
    ASSERT_EQ(implements->docs().commands().find("cmd")->second.size(), 1);
    EXPECT_EQ(implements->docs().commands().find("cmd")->second.back(), "value");
}

TEST_F(StructEntityTest, Nested_Structs_And_Enums_Share_Parent_Struct_Package_And_File)
{
    auto nestedStruct = structure_->addStruct("NestedStruct");
    auto nestedEnum = structure_->addEnum("NestedEnum");

    EXPECT_EQ(nestedStruct->package(), structure_->package());
    EXPECT_EQ(nestedStruct->file(), structure_->file());
    EXPECT_EQ(nestedEnum->package(), structure_->package());
    EXPECT_EQ(nestedEnum->file(), structure_->file());
}

TEST_F(StructEntityTest, addScalarField_Correctly_Initializes_And_Stores_Added_Field)
{
    Field* field = nullptr;

    ASSERT_TRUE(field = structure_->addScalarField("field",
                                                   13,
                                                   FieldTypeId::Int32,
                                                   FieldFlags::Observable | FieldFlags::Hashed,
                                                   "oneofName",
                                                   "1001",
                                                   {docs_.description()}));
    ASSERT_NE(structure_->fields().find("field"), structure_->fields().end());
    EXPECT_EQ(*(structure_->fields().find("field")), field);
    EXPECT_EQ(structure_->fields().size(), 1);

    EXPECT_EQ(field->type(), EntityTypeId::Field);
    EXPECT_EQ(field->name(), "field");
    EXPECT_EQ(field->dir(), structure_->dir());
    EXPECT_EQ(field->parent(), structure_);
    EXPECT_EQ(static_cast<const Field*>(field)->parent(), structure_);

    EXPECT_EQ(field->number(), 13);
    EXPECT_EQ(field->fieldType(), FieldTypeId::Int32);
    EXPECT_EQ(field->fieldTypeName(), GetFieldTypeIdStr(field->fieldType()));
    EXPECT_EQ(field->flags(), FieldFlags::Observable | FieldFlags::Hashed);
    EXPECT_FALSE(field->isOptional());
    EXPECT_FALSE(field->isRepeated());
    EXPECT_TRUE(field->isObservable());
    EXPECT_TRUE(field->isHashed());
    EXPECT_EQ(field->oneofName(), "oneofName");
    EXPECT_EQ(field->defaultValue(), "1001");

    EXPECT_EQ(field->docs().description(), docs_.description());
    EXPECT_EQ(field->docs().brief(), docs_.brief());
    EXPECT_TRUE(field->docs().commands().empty());
}

TEST_F(StructEntityTest, addStructField_Correctly_Initializes_And_Stores_Added_Field)
{
    Field* field = nullptr;

    ASSERT_TRUE(field = structure_->addStructField(
                    "field", 13, "MyStruct", FieldFlags::None, "oneofName", {docs_.description()}));
    ASSERT_NE(structure_->fields().find("field"), structure_->fields().end());
    EXPECT_EQ(*(structure_->fields().find("field")), field);
    EXPECT_EQ(structure_->fields().size(), 1);

    EXPECT_EQ(field->type(), EntityTypeId::Field);
    EXPECT_EQ(field->name(), "field");
    EXPECT_EQ(field->dir(), structure_->dir());
    EXPECT_EQ(field->parent(), structure_);
    EXPECT_EQ(static_cast<const Field*>(field)->parent(), structure_);

    EXPECT_EQ(field->number(), 13);
    EXPECT_EQ(field->fieldType(), FieldTypeId::Message);
    EXPECT_EQ(field->fieldTypeName(), "MyStruct");
    EXPECT_EQ(field->flags(), FieldFlags::None);
    EXPECT_FALSE(field->isOptional());
    EXPECT_FALSE(field->isRepeated());
    EXPECT_FALSE(field->isObservable());
    EXPECT_FALSE(field->isHashed());
    EXPECT_EQ(field->oneofName(), "oneofName");
    EXPECT_TRUE(field->defaultValue().empty());

    EXPECT_EQ(field->docs().description(), docs_.description());
    EXPECT_EQ(field->docs().brief(), docs_.brief());
    EXPECT_TRUE(field->docs().commands().empty());
}

TEST_F(StructEntityTest, addEnumField_Correctly_Initializes_And_Stores_Added_Field)
{
    Field* field = nullptr;

    ASSERT_TRUE(field =
                    structure_->addEnumField("field", 13, "MyEnum", FieldFlags::Repeated, "", {docs_.description()}));
    ASSERT_NE(structure_->fields().find("field"), structure_->fields().end());
    EXPECT_EQ(*(structure_->fields().find("field")), field);
    EXPECT_EQ(structure_->fields().size(), 1);

    EXPECT_EQ(field->type(), EntityTypeId::Field);
    EXPECT_EQ(field->name(), "field");
    EXPECT_EQ(field->dir(), structure_->dir());
    EXPECT_EQ(field->parent(), structure_);
    EXPECT_EQ(static_cast<const Field*>(field)->parent(), structure_);

    EXPECT_EQ(field->number(), 13);
    EXPECT_EQ(field->fieldType(), FieldTypeId::Enum);
    EXPECT_EQ(field->fieldTypeName(), "MyEnum");
    EXPECT_EQ(field->flags(), FieldFlags::Repeated);
    EXPECT_FALSE(field->isOptional());
    EXPECT_TRUE(field->isRepeated());
    EXPECT_FALSE(field->isObservable());
    EXPECT_FALSE(field->isHashed());
    EXPECT_TRUE(field->oneofName().empty());
    EXPECT_TRUE(field->defaultValue().empty());

    EXPECT_EQ(field->docs().description(), docs_.description());
    EXPECT_EQ(field->docs().brief(), docs_.brief());
    EXPECT_TRUE(field->docs().commands().empty());
}

TEST_F(StructEntityTest, addMapField_Correctly_Initializes_And_Stores_Added_Field)
{
    MapField* field = nullptr;

    ASSERT_TRUE(field = structure_->addMapField("field", 13, FieldTypeId::Fixed32, FieldTypeId::String));
    ASSERT_NE(structure_->fields().find("field"), structure_->fields().end());
    EXPECT_EQ(*(structure_->fields().find("field")), field);
    EXPECT_EQ(structure_->fields().size(), 1);

    EXPECT_EQ(field->type(), EntityTypeId::Field);
    EXPECT_EQ(field->name(), "field");
    EXPECT_EQ(field->dir(), structure_->dir());
    EXPECT_EQ(field->parent(), structure_);
    EXPECT_EQ(static_cast<const Field*>(field)->parent(), structure_);

    EXPECT_EQ(field->number(), 13);
    EXPECT_EQ(field->fieldType(), FieldTypeId::Map);
    EXPECT_EQ(field->fieldTypeName(), "map<fixed32, string>");
    EXPECT_EQ(field->flags(), FieldFlags::None);
    EXPECT_FALSE(field->isOptional());
    EXPECT_FALSE(field->isRepeated());
    EXPECT_FALSE(field->isObservable());
    EXPECT_FALSE(field->isHashed());
    EXPECT_TRUE(field->oneofName().empty());
    EXPECT_TRUE(field->defaultValue().empty());

    EXPECT_EQ(field->keyType(), FieldTypeId::Fixed32);
    EXPECT_EQ(field->keyTypeName(), GetFieldTypeIdStr(field->keyType()));
    EXPECT_EQ(field->valueType(), FieldTypeId::String);
    EXPECT_EQ(field->valueTypeName(), GetFieldTypeIdStr(FieldTypeId::String));

    EXPECT_TRUE(field->docs().description().empty());
    EXPECT_TRUE(field->docs().brief().empty());
    EXPECT_TRUE(field->docs().commands().empty());
}

TEST_F(StructEntityTest, Adding_Field_Throws_Entity_Error_If_Field_Number_Is_Invalid)
{
    EXPECT_ENTITY_EXCEPTION(structure_->addScalarField("field", Min_Field_Number - 1, FieldTypeId::Bool),
                            EntityTypeId::Struct,
                            structure_->dname());
    EXPECT_ENTITY_EXCEPTION(structure_->addStructField("field", Max_Field_Number + 1, "MyStruct"),
                            EntityTypeId::Struct,
                            structure_->dname());
    EXPECT_ENTITY_EXCEPTION(structure_->addEnumField("field", Reserved_Field_Number_Range_Start, "MyEnum"),
                            EntityTypeId::Struct,
                            structure_->dname());
    EXPECT_ENTITY_EXCEPTION(
        structure_->addMapField("field", Reserved_Field_Number_Range_End, FieldTypeId::Fixed32, FieldTypeId::Fixed32),
        EntityTypeId::Struct,
        structure_->dname());
}

TEST_F(StructEntityTest, Adding_Field_Throws_Entity_Error_If_Field_Flags_Are_Mutually_Exclusive)
{
    EXPECT_ENTITY_EXCEPTION(
        structure_->addScalarField("field", 1, FieldTypeId::Bool, FieldFlags::Optional | FieldFlags::Repeated),
        EntityTypeId::Struct,
        structure_->dname());
}

TEST_F(StructEntityTest, Adding_Field_Throws_Entity_Error_If_Field_Flags_Conflict_With_Oneof)
{
    EXPECT_ENTITY_EXCEPTION(
        structure_->addScalarField("field", 1, FieldTypeId::Bool, FieldFlags::Repeated, "oneofName"),
        EntityTypeId::Struct,
        structure_->dname());
    EXPECT_ENTITY_EXCEPTION(
        structure_->addScalarField("field", 1, FieldTypeId::Bool, FieldFlags::Optional, "oneofName"),
        EntityTypeId::Struct,
        structure_->dname());
}

TEST_F(StructEntityTest, Adding_Field_Throws_Entity_Error_If_Invalid_Custom_Typename_Is_Specified)
{
    EXPECT_ENTITY_EXCEPTION(structure_->addStructField("field", 1, ""), EntityTypeId::Struct, structure_->dname());
    EXPECT_ENTITY_EXCEPTION(structure_->addEnumField("field", 1, ""), EntityTypeId::Struct, structure_->dname());
    EXPECT_ENTITY_EXCEPTION(structure_->addMapField("field", 1, FieldTypeId::Int32, FieldTypeId::Message, ""),
                            EntityTypeId::Struct,
                            structure_->dname());
    EXPECT_ENTITY_EXCEPTION(structure_->addMapField("field", 1, FieldTypeId::Int32, FieldTypeId::Enum, ""),
                            EntityTypeId::Struct,
                            structure_->dname());

    EXPECT_ENTITY_EXCEPTION(structure_->addStructField("field", 1, "0a"), EntityTypeId::Struct, structure_->dname());
    EXPECT_ENTITY_EXCEPTION(structure_->addEnumField("field", 1, "a..b"), EntityTypeId::Struct, structure_->dname());
    EXPECT_ENTITY_EXCEPTION(structure_->addMapField("field", 1, FieldTypeId::Int32, FieldTypeId::Message, ".a.b"),
                            EntityTypeId::Struct,
                            structure_->dname());
    EXPECT_ENTITY_EXCEPTION(structure_->addMapField("field", 1, FieldTypeId::Int32, FieldTypeId::Enum, "a.b."),
                            EntityTypeId::Struct,
                            structure_->dname());
}

TEST_F(StructEntityTest, Adding_Field_Throws_Name_Conflict_Error_If_Field_Number_Duplicates_Another_One)
{
    ASSERT_TRUE(structure_->addScalarField("field1", 1, FieldTypeId::Int32));

    EXPECT_NAME_CONFLICT_EXCEPTION(structure_->addScalarField("field2", 1, FieldTypeId::Sint32),
                                   EntityTypeId::Struct,
                                   structure_->dname(),
                                   "field2");
    EXPECT_NAME_CONFLICT_EXCEPTION(
        structure_->addStructField("field3", 1, "MyStruct"), EntityTypeId::Struct, structure_->dname(), "field3");
    EXPECT_NAME_CONFLICT_EXCEPTION(
        structure_->addEnumField("field4", 1, "MyEnum"), EntityTypeId::Struct, structure_->dname(), "field4");
    EXPECT_NAME_CONFLICT_EXCEPTION(structure_->addMapField("field5", 1, FieldTypeId::Sint32, FieldTypeId::Sfixed32),
                                   EntityTypeId::Struct,
                                   structure_->dname(),
                                   "field5");
}

TEST_F(StructEntityTest, Structure_Is_Encodable_If_All_Fields_Are_Encodable)
{
    structure_->addScalarField(
        "field1", 1, FieldTypeId::Int32, FieldFlags::Optional | FieldFlags::Observable | FieldFlags::Hashed);
    structure_->addEnumField("field2", 2, "MyEnum", FieldFlags::Optional | FieldFlags::Observable | FieldFlags::Hashed);

    EXPECT_TRUE(structure_->isEncodable());
}

TEST_F(StructEntityTest, Structure_Is_Not_Encodable_If_Any_Field_Is_Not_Encodable)
{
    structure_->addScalarField(
        "field1", 1, FieldTypeId::Int32, FieldFlags::Optional | FieldFlags::Observable | FieldFlags::Hashed);
    structure_->addEnumField("field2", 2, "MyEnum", FieldFlags::Optional | FieldFlags::Observable | FieldFlags::Hashed);
    structure_->addStructField("field3", 3, "MyStruct");

    EXPECT_FALSE(structure_->isEncodable());
}
}} // namespace busrpc::test
