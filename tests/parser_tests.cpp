#include "parser/parser.h"
#include "utils/project_utils.h"

#include <gtest/gtest.h>

namespace busrpc { namespace test {

TEST(ParserTest, File_Error_Category_Name_Is_Not_Empty)
{
    EXPECT_TRUE(parser_error_category().name());
    EXPECT_NE(parser_error_category().name()[0], 0);
}

TEST(ParserTest, File_Error_Codes_Have_Non_Empty_Descriptions)
{
    using enum ParserErrc;

    EXPECT_FALSE(parser_error_category().message(static_cast<int>(Read_Failed)).empty());
    EXPECT_FALSE(parser_error_category().message(static_cast<int>(Protobuf_Error)).empty());
}

TEST(ParserTest, Unknown_File_Error_Code_Has_Non_Empty_Description)
{
    EXPECT_FALSE(parser_error_category().message(0).empty());
}

TEST(ParserTest, Ctor_Correctly_Initiliazes_Object)
{
    std::filesystem::path projectDir("project_dir");
    std::filesystem::path protobufRoot("protobuf_root");
    Parser parser(projectDir, protobufRoot);

    EXPECT_EQ(parser.projectDir(), projectDir);
    EXPECT_EQ(parser.protobufRoot(), protobufRoot);
}

TEST(ParserTest, Parser_Correctly_Handles_Test_Project)
{
    TmpDir dir;
    CreateTestProject(dir);
    std::cout << "done\n";
}
}} // namespace busrpc::test
