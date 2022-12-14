#include "entities/project.h"

#include <gtest/gtest.h>

namespace busrpc { namespace test {

class ClassEntityTest: public ::testing::Test {
protected:
    void SetUp() override
    {
        project_ = std::make_shared<Project>();
        auto api = project_->addApi();
        auto ns = api->addNamespace("namespace");

        ns_ = ns;
        cls_ = ns->addClass("class");
    }

protected:
    std::shared_ptr<Project> project_;
    const Namespace* ns_ = nullptr;
    Class* cls_ = nullptr;
};

TEST_F(ClassEntityTest, Class_Entity_Is_Correctly_Initialized_When_Created_By_Namespace_Entity)
{
    EXPECT_EQ(cls_->type(), EntityTypeId::Class);
    EXPECT_EQ(cls_->name(), "class");
    EXPECT_EQ(cls_->dir(), std::filesystem::path(Api_Entity_Name) / "namespace" / "class");
    EXPECT_EQ(cls_->dname(), std::string(Project_Entity_Name) + "." + Api_Entity_Name + ".namespace.class");
    EXPECT_TRUE(cls_->description().empty());
    EXPECT_TRUE(cls_->briefDescription().empty());
    EXPECT_TRUE(cls_->docCommands().empty());
    EXPECT_EQ(cls_->parent(), ns_);
    EXPECT_EQ(static_cast<const Class*>(cls_)->parent(), ns_);
    EXPECT_FALSE(cls_->descriptor());
    EXPECT_FALSE(cls_->objectId());
    EXPECT_TRUE(cls_->isStatic());
    EXPECT_TRUE(cls_->methods().empty());
}

TEST_F(ClassEntityTest, addMethod_Stores_Added_Method)
{
    Method* method = nullptr;

    EXPECT_TRUE(method = cls_->addMethod("method"));
    ASSERT_NE(cls_->methods().find("method"), cls_->methods().end());
    EXPECT_EQ(cls_->methods().find("method")->second, method);
    EXPECT_EQ(cls_->methods().size(), 1);
}

TEST_F(ClassEntityTest, Adding_ClassDesc_Struct_Sets_Class_Descriptor)
{
    Struct* desc = nullptr;

    EXPECT_TRUE(desc = cls_->addStruct(GetPredefinedStructName(StructTypeId::Class_Desc), "class.proto"));
    EXPECT_EQ(desc, cls_->descriptor());
}

TEST_F(ClassEntityTest, Adding_ClassDesc_Struct_Sets_Class_Documentation)
{
    std::string blockComment = "\\cmd cmd value\n"
                               "Brief description.\n";
    Struct* desc = nullptr;

    EXPECT_TRUE(desc = cls_->addStruct(
                    GetPredefinedStructName(StructTypeId::Class_Desc), "class.proto", StructFlags::None, blockComment));
    EXPECT_EQ(desc, cls_->descriptor());
    ASSERT_EQ(cls_->description().size(), 1);
    EXPECT_EQ(cls_->description()[0], "Brief description.");
    EXPECT_EQ(cls_->briefDescription(), "Brief description.");
    EXPECT_EQ(cls_->docCommands().size(), 1);
    ASSERT_NE(cls_->docCommands().find("cmd"), cls_->docCommands().end());
    EXPECT_EQ(cls_->docCommands().find("cmd")->second, "cmd value");
}

TEST_F(ClassEntityTest, Adding_ObjectId_Struct_To_Descriptor_Sets_Class_Object_Identifier)
{
    Struct* desc = nullptr;
    Struct* objectId = nullptr;

    ASSERT_TRUE(
        desc = cls_->addStruct(GetPredefinedStructName(StructTypeId::Class_Desc), "class.proto", StructFlags::None));
    EXPECT_TRUE(objectId = desc->addStruct(GetPredefinedStructName(StructTypeId::Object_Id)));
    EXPECT_EQ(cls_->objectId(), objectId);
    EXPECT_FALSE(cls_->isStatic());
}
}} // namespace busrpc::test
