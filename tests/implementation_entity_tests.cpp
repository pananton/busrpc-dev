#include "entities/project.h"

#include <gtest/gtest.h>

namespace busrpc { namespace test {

class ImplementationEntityTest: public ::testing::Test {
protected:
    void SetUp() override { implementation_ = project_.addImplementation(); }

protected:
    Project project_;
    Implementation* implementation_ = nullptr;
};

TEST_F(ImplementationEntityTest, Implementation_Entity_Is_Correctly_Initialized_When_Created_By_Project_Entity)
{
    EXPECT_EQ(implementation_->type(), EntityTypeId::Implementation);
    EXPECT_EQ(implementation_->name(), Implementation_Entity_Name);
    EXPECT_EQ(implementation_->dir(), std::filesystem::path(Implementation_Entity_Name));
    EXPECT_EQ(implementation_->dname(), std::string(Project_Entity_Name) + "." + Implementation_Entity_Name);
    EXPECT_EQ(implementation_->parent(), &project_);
    EXPECT_EQ(static_cast<const Implementation*>(implementation_)->parent(), &project_);
    ASSERT_EQ(implementation_->docs().description().size(), 1);
    EXPECT_EQ(implementation_->docs().description()[0], Implementation_Entity_Description);
}

TEST_F(ImplementationEntityTest, addServices_Stores_Added_Services)
{
    Service* service = nullptr;

    EXPECT_TRUE(service = implementation_->addService("service"));
    ASSERT_NE(implementation_->services().find("service"), implementation_->services().end());
    EXPECT_EQ(*(implementation_->services().find("service")), service);
    EXPECT_EQ(implementation_->services().size(), 1);
}
}} // namespace busrpc::test
