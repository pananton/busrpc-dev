#include "entities/project.h"
#include "utils/common.h"

#include <gtest/gtest.h>

namespace busrpc { namespace test {

class StructEntityTest: public ::testing::Test {
protected:
    void SetUp() override
    {
        project_ = std::make_shared<Project>();
        auto api = project_->addApi();
        auto ns = api->addNamespace("namespace");
        auto cls = ns->addClass("class");
        auto method = cls->addMethod("method");

        std::string blockComment = "\\cmd cmd value\n"
                                   "Brief description.\n"
                                   "Description.";
        method_ = method;
        structure_ = method->addStruct("Struct", "struct.proto", StructFlags::Hashed, blockComment);
    }

protected:
    std::shared_ptr<Project> project_;
    const Method* method_ = nullptr;
    Struct* structure_ = nullptr;
};

TEST_F(StructEntityTest, Struct_Entity_Is_Correctly_Initialized_When_Created_By_Parent_Entity_With_Non_Struct_Type)
{
    EXPECT_EQ(structure_->type(), EntityTypeId::Struct);
    EXPECT_EQ(structure_->name(), "Struct");
    EXPECT_EQ(structure_->dir(), std::filesystem::path(Api_Entity_Name) / "namespace" / "class" / "method");
    EXPECT_EQ(structure_->dname(),
              std::string(Project_Entity_Name) + "." + Api_Entity_Name + ".namespace.class.method.Struct");
    EXPECT_EQ(structure_->parent(), method_);
    EXPECT_EQ(static_cast<const Struct*>(structure_)->parent(), method_);
    EXPECT_EQ(structure_->package(),
              std::string(Project_Entity_Name) + "." + Api_Entity_Name + ".namespace.class.method");
    EXPECT_EQ(structure_->structType(), StructTypeId::General);
    EXPECT_EQ(structure_->file(), structure_->dir() / "struct.proto");
    EXPECT_EQ(structure_->flags(), StructFlags::Hashed);
    EXPECT_TRUE(structure_->isHashed());
    EXPECT_TRUE(structure_->fields().empty());

    ASSERT_EQ(structure_->description().size(), 2);
    EXPECT_EQ(structure_->description()[0], "Brief description.");
    EXPECT_EQ(structure_->description()[1], "Description.");
    EXPECT_EQ(structure_->briefDescription(), "Brief description.");
    EXPECT_EQ(structure_->docCommands().size(), 1);
    ASSERT_NE(structure_->docCommands().find("cmd"), structure_->docCommands().end());
    EXPECT_EQ(structure_->docCommands().find("cmd")->second, "cmd value");
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
                                                   FieldFlags::Optional | FieldFlags::Observable | FieldFlags::Hashed,
                                                   "oneofName",
                                                   "1001",
                                                   "Brief description."));
    ASSERT_NE(structure_->fields().find("field"), structure_->fields().end());
    EXPECT_EQ(structure_->fields().find("field")->second, field);
    EXPECT_EQ(structure_->fields().size(), 1);

    EXPECT_EQ(field->type(), EntityTypeId::Field);
    EXPECT_EQ(field->name(), "field");
    EXPECT_EQ(field->dir(), structure_->dir());
    EXPECT_EQ(field->parent(), structure_);
    EXPECT_EQ(static_cast<const Field*>(field)->parent(), structure_);

    EXPECT_EQ(field->number(), 13);
    EXPECT_EQ(field->fieldType(), FieldTypeId::Int32);
    EXPECT_EQ(field->fieldTypeName(), GetFieldTypeIdStr(field->fieldType()));
    EXPECT_EQ(field->flags(), FieldFlags::Optional | FieldFlags::Observable | FieldFlags::Hashed);
    EXPECT_TRUE(field->isOptional());
    EXPECT_FALSE(field->isRepeated());
    EXPECT_TRUE(field->isObservable());
    EXPECT_TRUE(field->isHashed());
    EXPECT_EQ(field->oneofName(), "oneofName");
    EXPECT_EQ(field->defaultValue(), "1001");

    ASSERT_EQ(field->description().size(), 1);
    EXPECT_EQ(field->description()[0], "Brief description.");
    EXPECT_EQ(field->briefDescription(), "Brief description.");
    EXPECT_TRUE(field->docCommands().empty());
}

TEST_F(StructEntityTest, addStructField_Correctly_Initializes_And_Stores_Added_Field)
{
    Field* field = nullptr;

    ASSERT_TRUE(field = structure_->addStructField(
                    "field", 13, "MyStruct", FieldFlags::None, "oneofName", "Brief description."));
    ASSERT_NE(structure_->fields().find("field"), structure_->fields().end());
    EXPECT_EQ(structure_->fields().find("field")->second, field);
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

    ASSERT_EQ(field->description().size(), 1);
    EXPECT_EQ(field->description()[0], "Brief description.");
    EXPECT_EQ(field->briefDescription(), "Brief description.");
    EXPECT_TRUE(field->docCommands().empty());
}

TEST_F(StructEntityTest, addEnumField_Correctly_Initializes_And_Stores_Added_Field)
{
    Field* field = nullptr;

    ASSERT_TRUE(field = structure_->addEnumField("field", 13, "MyEnum", FieldFlags::Repeated, "", "\\cmd cmd value"));
    ASSERT_NE(structure_->fields().find("field"), structure_->fields().end());
    EXPECT_EQ(structure_->fields().find("field")->second, field);
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

    EXPECT_TRUE(field->description().empty());
    EXPECT_TRUE(field->briefDescription().empty());
    ASSERT_NE(field->docCommands().find("cmd"), field->docCommands().end());
    EXPECT_EQ(field->docCommands().find("cmd")->second, "cmd value");
    EXPECT_EQ(field->docCommands().size(), 1);
}

TEST_F(StructEntityTest, addMapField_Correctly_Initializes_And_Stores_Added_Field)
{
    MapField* field = nullptr;

    ASSERT_TRUE(field = structure_->addMapField("field", 13, FieldTypeId::Fixed32, FieldTypeId::String));
    ASSERT_NE(structure_->fields().find("field"), structure_->fields().end());
    EXPECT_EQ(structure_->fields().find("field")->second, field);
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
    EXPECT_EQ(field->valueType(), FieldTypeId::String);
    EXPECT_EQ(field->valueTypeName(), GetFieldTypeIdStr(FieldTypeId::String));

    EXPECT_TRUE(field->description().empty());
    EXPECT_TRUE(field->briefDescription().empty());
    EXPECT_TRUE(field->docCommands().empty());
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
    EXPECT_ENTITY_EXCEPTION(
        structure_->addScalarField("field", 1, FieldTypeId::Bool, FieldFlags::Repeated | FieldFlags::Observable),
        EntityTypeId::Struct,
        structure_->dname());
    EXPECT_ENTITY_EXCEPTION(
        structure_->addScalarField("field", 1, FieldTypeId::Bool, FieldFlags::Repeated | FieldFlags::Hashed),
        EntityTypeId::Struct,
        structure_->dname());
}

TEST_F(StructEntityTest, Adding_Field_Throws_Entity_Error_If_Field_Flags_Conflict_With_Oneof)
{
    EXPECT_ENTITY_EXCEPTION(
        structure_->addScalarField("field", 1, FieldTypeId::Bool, FieldFlags::Repeated, "oneofName"),
        EntityTypeId::Struct,
        structure_->dname());
}

TEST_F(StructEntityTest, Adding_Field_Throws_Entity_Error_If_Typename_Is_Not_Specified_For_Field_With_Custom_Type)
{
    EXPECT_ENTITY_EXCEPTION(structure_->addStructField("field", 1, ""), EntityTypeId::Struct, structure_->dname());
    EXPECT_ENTITY_EXCEPTION(structure_->addEnumField("field", 1, ""), EntityTypeId::Struct, structure_->dname());
    EXPECT_ENTITY_EXCEPTION(structure_->addMapField("field", 1, FieldTypeId::Int32, FieldTypeId::Message, ""),
                            EntityTypeId::Struct,
                            structure_->dname());
    EXPECT_ENTITY_EXCEPTION(structure_->addMapField("field", 1, FieldTypeId::Int32, FieldTypeId::Enum, ""),
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
}} // namespace busrpc::test
