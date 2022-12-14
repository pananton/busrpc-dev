#include "entities/project.h"

#include <gtest/gtest.h>

namespace busrpc { namespace test {

class EnumEntityTest: public ::testing::Test {
protected:
    void SetUp() override
    {
        project_ = std::make_shared<Project>();
        auto api = project_->addApi();
        std::string blockComment = "\\cmd cmd value\n"
                                   "Brief description.\n"
                                   "Description.";
        api_ = api;
        enum_ = api->addEnum("Enum", "enum.proto", blockComment);
    }

protected:
    std::shared_ptr<Project> project_;
    const Api* api_ = nullptr;
    Enum* enum_ = nullptr;
};

TEST_F(EnumEntityTest, Enum_Entity_Is_Correctly_Initialized_When_Created_By_Parent_Entity)
{
    EXPECT_EQ(enum_->type(), EntityTypeId::Enum);
    EXPECT_EQ(enum_->name(), "Enum");
    EXPECT_EQ(enum_->dir(), std::filesystem::path(Api_Entity_Name));
    EXPECT_EQ(enum_->dname(), std::string(Project_Entity_Name) + "." + Api_Entity_Name + ".Enum");
    EXPECT_EQ(enum_->parent(), api_);
    EXPECT_EQ(static_cast<const Enum*>(enum_)->parent(), api_);
    EXPECT_EQ(enum_->package(), std::string(Project_Entity_Name) + "." + Api_Entity_Name);
    EXPECT_EQ(enum_->file(), enum_->dir() / "enum.proto");
    EXPECT_TRUE(enum_->constants().empty());

    ASSERT_EQ(enum_->description().size(), 2);
    EXPECT_EQ(enum_->description()[0], "Brief description.");
    EXPECT_EQ(enum_->description()[1], "Description.");
    EXPECT_EQ(enum_->briefDescription(), "Brief description.");
    EXPECT_EQ(enum_->docCommands().size(), 1);
    ASSERT_NE(enum_->docCommands().find("cmd"), enum_->docCommands().end());
    EXPECT_EQ(enum_->docCommands().find("cmd")->second, "cmd value");
}

TEST_F(EnumEntityTest, addConstant_Correctly_Initializes_And_Stores_Added_Constant)
{
    Constant* constant = nullptr;

    ASSERT_TRUE(constant = enum_->addConstant("constant", 13, "Brief description."));
    ASSERT_NE(enum_->constants().find("constant"), enum_->constants().end());
    ASSERT_EQ(enum_->constants().find("constant")->second, constant);

    EXPECT_EQ(constant->type(), EntityTypeId::Constant);
    EXPECT_EQ(constant->name(), "constant");
    EXPECT_EQ(constant->dir(), enum_->dir());
    EXPECT_EQ(constant->parent(), enum_);
    EXPECT_EQ(static_cast<const Constant*>(constant)->parent(), enum_);
    EXPECT_EQ(constant->value(), 13);

    ASSERT_EQ(constant->description().size(), 1);
    EXPECT_EQ(constant->description()[0], "Brief description.");
    EXPECT_EQ(constant->briefDescription(), "Brief description.");
    EXPECT_TRUE(constant->docCommands().empty());
}
}} // namespace busrpc::test
