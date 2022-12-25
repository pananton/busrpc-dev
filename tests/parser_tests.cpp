#include "parser/parser.h"

#include <fstream>
#include <gtest/gtest.h>

#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/tokenizer.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

namespace protobuf = google::protobuf;

namespace busrpc { namespace test {

TEST(ParserTest, File_Error_Category_Name_Is_Not_Empty)
{
    EXPECT_TRUE(file_error_category().name());
    EXPECT_NE(file_error_category().name()[0], 0);
}

TEST(ParserTest, File_Error_Codes_Have_Non_Empty_Descriptions)
{
    using enum FileErrc;

    EXPECT_FALSE(file_error_category().message(static_cast<int>(Read_Failed)).empty());
    EXPECT_FALSE(file_error_category().message(static_cast<int>(Parser_Error)).empty());
}

TEST(ParserTest, Unknown_File_Error_Code_Has_Non_Empty_Description)
{
    EXPECT_FALSE(file_error_category().message(0).empty());
}

TEST(ParserTest, Ctor_Correctly_Initiliazes_Object)
{
    std::filesystem::path projectDir("project_dir");
    std::filesystem::path protobufRoot("protobuf_root");
    Parser parser(projectDir, protobufRoot);

    EXPECT_EQ(parser.projectDir(), projectDir);
    EXPECT_EQ(parser.protobufRoot(), protobufRoot);
}
}} // namespace busrpc::test
