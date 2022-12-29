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
    EXPECT_TRUE(cls_->docs().description().empty());
    EXPECT_TRUE(cls_->docs().brief().empty());
    EXPECT_TRUE(cls_->docs().commands().empty());
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
    EXPECT_EQ(*(cls_->methods().find("method")), method);
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
    EntityDocs docs({"Brief description."}, {{"cmd", {"cmd value"}}});
    Struct* desc = nullptr;

    EXPECT_TRUE(desc = cls_->addStruct(
                    GetPredefinedStructName(StructTypeId::Class_Desc), "class.proto", StructFlags::None, docs));
    EXPECT_EQ(desc, cls_->descriptor());
    EXPECT_EQ(cls_->docs().description(), docs.description());
    EXPECT_EQ(cls_->docs().brief(), docs.brief());
    EXPECT_EQ(cls_->docs().commands(), docs.commands());
}

TEST_F(ClassEntityTest, Adding_ObjectId_Struct_To_Descriptor_Sets_Class_Object_Identifier)
{
    Struct* desc = nullptr;
    Struct* objectId = nullptr;

    ASSERT_TRUE(
        desc = cls_->addStruct(GetPredefinedStructName(StructTypeId::Class_Desc), "class.proto", StructFlags::None));
    EXPECT_TRUE(objectId = desc->addStruct(GetPredefinedStructName(StructTypeId::Class_Object_Id)));
    EXPECT_EQ(cls_->objectId(), objectId);
    EXPECT_FALSE(cls_->isStatic());
}
}} // namespace busrpc::test
