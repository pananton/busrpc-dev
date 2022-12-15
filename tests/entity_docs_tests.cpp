#include "entities/entity.h"

#include <gtest/gtest.h>

namespace busrpc { namespace test {

TEST(CommonEntityTest, Entity_Docs_Ctor_Correctly_Parses_Empty_Block_Comment)
{
    EntityDocs docs("");

    EXPECT_TRUE(docs.description().empty());
    EXPECT_TRUE(docs.brief().empty());
    EXPECT_TRUE(docs.commands().empty());
}

TEST(CommonEntityTest, Entity_Docs_Ctor_Correctly_Initializes_Object_From_Block_Comment)
{
    std::string blockComment = "\\cmd1\tFirst cmd1 instance   \n"
                               "\tThis is a brief description. \n"
                               " This is the first line of long description.\t\n"
                               "\\cmd2\n"
                               "\\\n"
                               "\\ \t\n"
                               " \\ Empty command value\n"
                               "This is the second line of long description,\n"
                               "\n"
                               " \t \n"
                               "which continues on the third line.\n"
                               "  \t  \\cmd1 Second cmd1 instance\t";
    EntityDocs docs(blockComment);
    std::vector<std::string> cmd1Value = {"First cmd1 instance", "Second cmd1 instance"};
    std::vector<std::string> cmd2Value = {""};
    std::vector<std::string> emptyCmdValue = {"", "", "Empty command value"};

    EXPECT_EQ(docs.brief(), "\tThis is a brief description. ");

    ASSERT_EQ(docs.description().size(), 6);
    EXPECT_EQ(docs.description()[0], docs.brief());
    EXPECT_EQ(docs.description()[1], " This is the first line of long description.\t");
    EXPECT_EQ(docs.description()[2], "This is the second line of long description,");
    EXPECT_EQ(docs.description()[3], "");
    EXPECT_EQ(docs.description()[4], " \t ");
    EXPECT_EQ(docs.description()[5], "which continues on the third line.");

    EXPECT_EQ(docs.commands().size(), 3);
    ASSERT_NE(docs.commands().find("cmd1"), docs.commands().end());
    EXPECT_EQ(docs.commands().find("cmd1")->second, cmd1Value);
    ASSERT_NE(docs.commands().find("cmd2"), docs.commands().end());
    EXPECT_EQ(docs.commands().find("cmd2")->second, cmd2Value);
    ASSERT_NE(docs.commands().find(""), docs.commands().end());
    EXPECT_EQ(docs.commands().find("")->second, emptyCmdValue);
}

TEST(CommonEntityTest, Entity_Docs_Ctor_Correctly_Initializes_Object_From_Description_And_Doc_Commands)
{
    std::vector<std::string> description = {"Brief description.", "Description."};
    std::vector<std::string> cmd1Value = {"value1", "value2"};
    std::vector<std::string> cmd2Value = {"value3"};
    std::vector<std::string> cmd3Value = {};

    EntityDocs docs(description, {{"cmd1", cmd1Value}, {"cmd2", cmd2Value}, {"cmd3", cmd3Value}});

    EXPECT_EQ(docs.description(), description);
    EXPECT_EQ(docs.brief(), description[0]);
    ASSERT_NE(docs.commands().find("cmd1"), docs.commands().end());
    EXPECT_EQ(docs.commands().find("cmd1")->second, cmd1Value);
    ASSERT_NE(docs.commands().find("cmd2"), docs.commands().end());
    EXPECT_EQ(docs.commands().find("cmd2")->second, cmd2Value);
    ASSERT_NE(docs.commands().find("cmd3"), docs.commands().end());
    ASSERT_EQ(docs.commands().find("cmd3")->second.size(), 1); // ctor should add one empty value!
    EXPECT_EQ(docs.commands().find("cmd3")->second[0], "");
}
}} // namespace busrpc::test
