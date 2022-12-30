#include "entities/project.h"
#include "utils/common.h"

#include <gtest/gtest.h>

namespace busrpc { namespace test {

class ApiEntityTest: public ::testing::Test {
protected:
    void SetUp() override
    {
        project_ = std::make_shared<Project>();
        api_ = project_->addApi();
    }

protected:
    std::shared_ptr<Project> project_;
    Api* api_ = nullptr;
};

TEST_F(ApiEntityTest, Api_Entity_Is_Correctly_Initialized_When_Created_By_Project_Entity)
{
    EXPECT_EQ(api_->type(), EntityTypeId::Api);
    EXPECT_EQ(api_->name(), Api_Entity_Name);
    EXPECT_EQ(api_->dir(), Api_Entity_Name);
    EXPECT_EQ(api_->dname(), JoinStrings(Project_Entity_Name, ".", Api_Entity_Name));
    EXPECT_EQ(api_->parent(), project_.get());
    EXPECT_EQ(static_cast<const Api*>(api_)->parent(), project_.get());
    ASSERT_EQ(api_->docs().description().size(), 1);
    EXPECT_EQ(api_->docs().description()[0], Api_Entity_Description);
}

TEST_F(ApiEntityTest, addNamespace_Stores_Added_Namespace)
{
    Namespace* ns = nullptr;

    EXPECT_TRUE(ns = api_->addNamespace("namespace"));
    ASSERT_NE(api_->namespaces().find("namespace"), api_->namespaces().end());
    ASSERT_EQ(*(api_->namespaces().find("namespace")), ns);
}
}} // namespace busrpc::test
