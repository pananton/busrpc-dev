#include "entities/project.h"

#include <gtest/gtest.h>

namespace busrpc { namespace test {

class MethodEntityTest: public ::testing::Test {
protected:
    void SetUp() override
    {
        project_ = std::make_shared<Project>();
        auto api = project_->addApi();
        auto ns = api->addNamespace("namespace");
        auto cls = ns->addClass("class");

        cls_ = cls;
        method_ = cls->addMethod("method");
    }

protected:
    std::shared_ptr<Project> project_;
    const Class* cls_ = nullptr;
    Method* method_ = nullptr;
};

TEST_F(MethodEntityTest, Method_Entity_Is_Correctly_Initialized_When_Created_By_Class_Entity)
{
    EXPECT_EQ(method_->type(), EntityTypeId::Method);
    EXPECT_EQ(method_->name(), "method");
    EXPECT_EQ(method_->dir(), std::filesystem::path(Api_Entity_Name) / "namespace" / "class" / "method");
    EXPECT_EQ(method_->dname(), std::string(Project_Entity_Name) + "." + Api_Entity_Name + ".namespace.class.method");
    EXPECT_TRUE(method_->docs().description().empty());
    EXPECT_TRUE(method_->docs().brief().empty());
    EXPECT_TRUE(method_->docs().commands().empty());
    EXPECT_EQ(method_->parent(), cls_);
    EXPECT_EQ(static_cast<const Method*>(method_)->parent(), cls_);
    EXPECT_FALSE(method_->descriptor());
    EXPECT_FALSE(method_->params());
    EXPECT_FALSE(method_->retval());
    EXPECT_FALSE(method_->isStatic());
    EXPECT_FALSE(method_->hasParams());
    EXPECT_TRUE(method_->isOneway());
    EXPECT_FALSE(method_->isStatic());
}

TEST_F(MethodEntityTest, Adding_MethodDesc_Struct_Sets_Method_Descriptor)
{
    Struct* desc = nullptr;

    EXPECT_TRUE(desc = method_->addStruct(GetPredefinedStructName(StructTypeId::Method_Desc), "method.proto"));
    EXPECT_EQ(desc, method_->descriptor());
}

TEST_F(MethodEntityTest, Adding_MethodDesc_Struct_Sets_Method_Documentation)
{
    EntityDocs docs({}, {{"cmd", {"cmd value"}}});
    Struct* desc = nullptr;

    EXPECT_TRUE(
        desc = method_->addStruct(
            GetPredefinedStructName(StructTypeId::Method_Desc), "method.proto", StructFlags::None, docs));
    EXPECT_EQ(desc, method_->descriptor());
    EXPECT_EQ(method_->docs().description(), docs.description());
    EXPECT_EQ(method_->docs().brief(), docs.brief());
    EXPECT_EQ(method_->docs().commands(), docs.commands());
}

TEST_F(MethodEntityTest, Adding_Params_Struct_To_Descriptor_Sets_Method_Parameters)
{
    Struct* desc = nullptr;
    Struct* params = nullptr;

    ASSERT_TRUE(desc = method_->addStruct(
                    GetPredefinedStructName(StructTypeId::Method_Desc), "method.proto", StructFlags::None));
    EXPECT_TRUE(params = desc->addStruct(GetPredefinedStructName(StructTypeId::Method_Params)));
    EXPECT_EQ(method_->params(), params);
    EXPECT_TRUE(method_->hasParams());
}

TEST_F(MethodEntityTest, Adding_Retval_Struct_To_Descriptor_Sets_Method_Return_Value)
{
    Struct* desc = nullptr;
    Struct* retval = nullptr;

    ASSERT_TRUE(desc = method_->addStruct(
                    GetPredefinedStructName(StructTypeId::Method_Desc), "method.proto", StructFlags::None));
    EXPECT_TRUE(retval = desc->addStruct(GetPredefinedStructName(StructTypeId::Method_Retval)));
    EXPECT_EQ(method_->retval(), retval);
    EXPECT_FALSE(method_->isOneway());
}

TEST_F(MethodEntityTest, Adding_Static_Struct_To_Descriptor_Makes_Method_Static)
{
    Struct* desc = nullptr;
    Struct* staticMarker = nullptr;

    ASSERT_TRUE(desc = method_->addStruct(
                    GetPredefinedStructName(StructTypeId::Method_Desc), "method.proto", StructFlags::None));
    EXPECT_TRUE(staticMarker = desc->addStruct(GetPredefinedStructName(StructTypeId::Static_Marker)));
    EXPECT_TRUE(method_->isStatic());
}
}} // namespace busrpc::test
