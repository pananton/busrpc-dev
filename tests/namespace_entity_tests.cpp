#include "entities/project.h"

#include <gtest/gtest.h>

namespace busrpc { namespace test {

class NamespaceEntityTest: public ::testing::Test {
protected:
    void SetUp() override
    {
        project_ = std::make_shared<Project>();
        auto api = project_->addApi();

        api_ = api;
        ns_ = api->addNamespace("namespace");
    }

protected:
    std::shared_ptr<Project> project_;
    const Api* api_ = nullptr;
    Namespace* ns_ = nullptr;
};

TEST_F(NamespaceEntityTest, Namespace_Entity_Is_Correctly_Initialized_When_Created_By_Api_Entity)
{
    EXPECT_EQ(ns_->type(), EntityTypeId::Namespace);
    EXPECT_EQ(ns_->name(), "namespace");
    EXPECT_EQ(ns_->dir(), std::filesystem::path(Api_Entity_Name) / "namespace");
    EXPECT_EQ(ns_->dname(), std::string(Project_Entity_Name) + "." + Api_Entity_Name + ".namespace");
    EXPECT_TRUE(ns_->description().empty());
    EXPECT_TRUE(ns_->briefDescription().empty());
    EXPECT_TRUE(ns_->docCommands().empty());
    EXPECT_EQ(ns_->parent(), api_);
    EXPECT_EQ(static_cast<const Namespace*>(ns_)->parent(), api_);
    EXPECT_FALSE(ns_->descriptor());
    EXPECT_TRUE(ns_->classes().empty());
}

TEST_F(NamespaceEntityTest, addClass_Stores_Added_Class)
{
    Class* cls = nullptr;

    EXPECT_TRUE(cls = ns_->addClass("class"));
    ASSERT_NE(ns_->classes().find("class"), ns_->classes().end());
    EXPECT_EQ(ns_->classes().find("class")->second, cls);
    EXPECT_EQ(ns_->classes().size(), 1);
}

TEST_F(NamespaceEntityTest, Adding_NamespaceDesc_Struct_Sets_Namespace_Descriptor)
{
    Struct* desc = nullptr;

    EXPECT_TRUE(desc = ns_->addStruct(GetPredefinedStructName(StructTypeId::Namespace_Desc), "namespace.proto"));
    EXPECT_EQ(desc, ns_->descriptor());
}

TEST_F(NamespaceEntityTest, Adding_NamespaceDesc_Struct_Sets_Namespace_Documentation)
{
    std::string blockComment = "\\cmd cmd value\n"
                               "Brief description.\n"
                               "Description.";
    Struct* desc = nullptr;

    EXPECT_TRUE(
        desc = ns_->addStruct(
            GetPredefinedStructName(StructTypeId::Namespace_Desc), "namespace.proto", StructFlags::None, blockComment));
    EXPECT_EQ(desc, ns_->descriptor());
    ASSERT_EQ(ns_->description().size(), 2);
    EXPECT_EQ(ns_->description()[0], "Brief description.");
    EXPECT_EQ(ns_->description()[1], "Description.");
    EXPECT_EQ(ns_->briefDescription(), "Brief description.");
    EXPECT_EQ(ns_->docCommands().size(), 1);
    ASSERT_NE(ns_->docCommands().find("cmd"), ns_->docCommands().end());
    EXPECT_EQ(ns_->docCommands().find("cmd")->second, "cmd value");
}
}} // namespace busrpc::test
