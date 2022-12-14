#include "entities/project.h"

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
    EXPECT_EQ(api_->dname(), std::string(Project_Entity_Name) + "." + Api_Entity_Name);
    EXPECT_EQ(api_->parent(), project_.get());
    EXPECT_EQ(static_cast<const Api*>(api_)->parent(), project_.get());

    EXPECT_FALSE(api_->description().empty());
    EXPECT_FALSE(api_->briefDescription().empty());
    ASSERT_EQ(api_->description().size(), 1);
    EXPECT_EQ(api_->description()[0], Api_Entity_Comment);
}

TEST_F(ApiEntityTest, addNamespace_Stores_Added_Namespace)
{
    Namespace* ns = nullptr;

    EXPECT_TRUE(ns = api_->addNamespace("namespace"));
    ASSERT_NE(api_->namespaces().find("namespace"), api_->namespaces().end());
    ASSERT_EQ(api_->namespaces().find("namespace")->second, ns);
}

TEST_F(ApiEntityTest, Adding_CallMessage_Struct_Sets_Api_Call_Message_Type)
{
    Struct* callMsg = nullptr;

    EXPECT_TRUE(callMsg = api_->addStruct(GetPredefinedStructName(StructTypeId::Call_Message), "file.proto"));
    EXPECT_EQ(callMsg, api_->callMessage());
}

TEST_F(ApiEntityTest, Adding_ResultMessage_Struct_Sets_Api_Result_Message_Type)
{
    Struct* resultMsg = nullptr;

    EXPECT_TRUE(resultMsg = api_->addStruct(GetPredefinedStructName(StructTypeId::Result_Message), "file.proto"));
    EXPECT_EQ(resultMsg, api_->resultMessage());
}

TEST_F(ApiEntityTest, Adding_Exception_Struct_Sets_Api_Exception_Type)
{
    Struct* exception = nullptr;

    EXPECT_TRUE(exception = api_->addStruct(GetPredefinedStructName(StructTypeId::Method_Exception), "file.proto"));
    EXPECT_EQ(exception, api_->exception());
}

TEST_F(ApiEntityTest, Adding_Errc_Enum_Sets_Api_Error_Code_Type)
{
    Enum* errc = nullptr;

    EXPECT_TRUE(errc = api_->addEnum(Errc_Enumeration_Name, "file.proto"));
    EXPECT_EQ(errc, api_->errc());
}
}} // namespace busrpc::test
