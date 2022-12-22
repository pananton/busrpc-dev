#include "entities/project.h"

#include <gtest/gtest.h>

namespace busrpc { namespace test {

class ServicesEntityTest: public ::testing::Test {
protected:
    void SetUp() override
    {
        services_ = project_.addServices();
    }

protected:
    Project project_;
    Services* services_ = nullptr;
};

TEST_F(ServicesEntityTest, Services_Entity_Is_Correctly_Initialized_When_Created_By_Project_Entity)
{
    EXPECT_EQ(services_->type(), EntityTypeId::Services);
    EXPECT_EQ(services_->name(), Services_Entity_Name);
    EXPECT_EQ(services_->dir(), std::filesystem::path(Services_Entity_Name));
    EXPECT_EQ(services_->dname(), std::string(Project_Entity_Name) + "." + Services_Entity_Name);
    EXPECT_EQ(services_->parent(), &project_);
    EXPECT_EQ(static_cast<const Services*>(services_)->parent(), &project_);
    ASSERT_EQ(services_->docs().description().size(), 1);
    EXPECT_EQ(services_->docs().description()[0], Services_Entity_Description);
}

TEST_F(ServicesEntityTest, addServices_Stores_Added_Services)
{
    Service* service = nullptr;

    EXPECT_TRUE(service = services_->addService("service"));
    ASSERT_NE(services_->services().find("service"), services_->services().end());
    EXPECT_EQ(*(services_->services().find("service")), service);
    EXPECT_EQ(services_->services().size(), 1);
}
}} // namespace busrpc::test
