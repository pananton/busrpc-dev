#include "entities/project.h"

#include <gtest/gtest.h>

namespace busrpc { namespace test {

class EnumEntityTest: public ::testing::Test {
protected:
    void SetUp() override
    {
        docs_ = EntityDocs({"Brief description.", "Description"}, {{"cmd", {"cmd value"}}});
        auto api = project_.addApi();

        api_ = api;
        enum_ = api->addEnum("Enum", "enum.proto", docs_);
    }

protected:
    Project project_;
    EntityDocs docs_;
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

    EXPECT_EQ(enum_->docs().description(), docs_.description());
    EXPECT_EQ(enum_->docs().brief(), docs_.brief());
    EXPECT_EQ(enum_->docs().commands(), docs_.commands());
}

TEST_F(EnumEntityTest, addConstant_Correctly_Initializes_And_Stores_Added_Constant)
{
    Constant* constant = nullptr;

    ASSERT_TRUE(constant = enum_->addConstant("constant", 13, {docs_.description()}));
    ASSERT_NE(enum_->constants().find("constant"), enum_->constants().end());
    ASSERT_EQ(*(enum_->constants().find("constant")), constant);

    EXPECT_EQ(constant->type(), EntityTypeId::Constant);
    EXPECT_EQ(constant->name(), "constant");
    EXPECT_EQ(constant->dir(), enum_->dir());
    EXPECT_EQ(constant->parent(), enum_);
    EXPECT_EQ(static_cast<const Constant*>(constant)->parent(), enum_);
    EXPECT_EQ(constant->value(), 13);

    EXPECT_EQ(constant->docs().description(), docs_.description());
    EXPECT_EQ(constant->docs().brief(), docs_.brief());
    EXPECT_TRUE(constant->docs().commands().empty());
}
}} // namespace busrpc::test
