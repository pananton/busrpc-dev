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
    EXPECT_TRUE(ns_->docs().description().empty());
    EXPECT_TRUE(ns_->docs().brief().empty());
    EXPECT_TRUE(ns_->docs().commands().empty());
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
    EXPECT_EQ(*(ns_->classes().find("class")), cls);
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
    EntityDocs docs({"Brief description.", "Description"}, {{"cmd", {"cmd value"}}});
    Struct* desc = nullptr;

    EXPECT_TRUE(desc = ns_->addStruct(
                    GetPredefinedStructName(StructTypeId::Namespace_Desc), "namespace.proto", StructFlags::None, docs));
    EXPECT_EQ(desc, ns_->descriptor());
    EXPECT_EQ(ns_->docs().description(), docs.description());
    EXPECT_EQ(ns_->docs().brief(), docs.brief());
    EXPECT_EQ(ns_->docs().commands(), docs.commands());
}
}} // namespace busrpc::test
