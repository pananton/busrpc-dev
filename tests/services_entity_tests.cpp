#include "entities/project.h"

#include <gtest/gtest.h>

namespace busrpc { namespace test {

class ServicesEntityTest: public ::testing::Test {
protected:
    void SetUp() override
    {
        project_ = std::make_shared<Project>();
        services_ = project_->addServices();
    }

protected:
    std::shared_ptr<Project> project_;
    Services* services_ = nullptr;
};

TEST_F(ServicesEntityTest, Services_Entity_Is_Correctly_Initialized_When_Created_By_Project_Entity)
{
    EXPECT_EQ(services_->type(), EntityTypeId::Services);
    EXPECT_EQ(services_->name(), Services_Entity_Name);
    EXPECT_EQ(services_->dir(), std::filesystem::path(Services_Entity_Name));
    EXPECT_EQ(services_->dname(), std::string(Project_Entity_Name) + "." + Services_Entity_Name);
    EXPECT_EQ(services_->parent(), project_.get());
    EXPECT_EQ(static_cast<const Services*>(services_)->parent(), project_.get());

    EXPECT_FALSE(services_->description().empty());
    EXPECT_FALSE(services_->briefDescription().empty());
    ASSERT_EQ(services_->description().size(), 1);
    EXPECT_EQ(services_->description()[0], Services_Entity_Comment);
}

TEST_F(ServicesEntityTest, addServices_Stores_Added_Services)
{
    Service* service = nullptr;

    EXPECT_TRUE(service = services_->addService("service"));
    ASSERT_NE(services_->services().find("service"), services_->services().end());
    EXPECT_EQ(services_->services().find("service")->second, service);
    EXPECT_EQ(services_->services().size(), 1);
}
}} // namespace busrpc::test
