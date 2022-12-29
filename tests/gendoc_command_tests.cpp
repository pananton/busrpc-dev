#include "app.h"
#include "commands/gendoc/gendoc_command.h"
#include "commands/help/help_command.h"
#include "tests_configure.h"
#include "utils/common.h"
#include "utils/project_utils.h"

#include <CLI/CLI.hpp>
#include <gtest/gtest.h>

#include <sstream>

namespace busrpc { namespace test {

TEST(GenDocCommandTest, Command_Name_And_Id_Are_Mapped_To_Each_Other)
{
    EXPECT_EQ(CommandId::GenDoc, GetCommandId(GetCommandName(CommandId::GenDoc)));
    EXPECT_EQ(GenDocCommand::Id, CommandId::GenDoc);
    EXPECT_STREQ(GenDocCommand::Name, GetCommandName(CommandId::GenDoc));
}

TEST(GenDocCommandTest, Command_Error_Category_Name_Matches_Command_Name)
{
    EXPECT_STREQ(gendoc_error_category().name(), GetCommandName(CommandId::GenDoc));
}

TEST(GenDocCommandTest, Description_For_Unknown_Command_Error_Code_Is_Not_Empty)
{
    EXPECT_FALSE(gendoc_error_category().message(0).empty());
}

TEST(GenDocCommandTest, Description_For_Unknown_Command_Error_Code_Differs_From_Known_Error_Codes_Descriptions)
{
    using enum GenDocErrc;

    EXPECT_NE(gendoc_error_category().message(static_cast<int>(Spec_Violated)), gendoc_error_category().message(0));
    EXPECT_NE(gendoc_error_category().message(static_cast<int>(Protobuf_Parsing_Failed)),
              gendoc_error_category().message(0));
    EXPECT_NE(gendoc_error_category().message(static_cast<int>(File_Read_Failed)), gendoc_error_category().message(0));
    EXPECT_NE(gendoc_error_category().message(static_cast<int>(File_Write_Failed)), gendoc_error_category().message(0));
    EXPECT_NE(gendoc_error_category().message(static_cast<int>(Invalid_Project_Dir)),
              gendoc_error_category().message(0));
}

TEST(GenDocCommandTest, Error_Codes_Are_Mapped_To_Appropriate_Error_Conditions)
{
    using enum GenDocErrc;

    EXPECT_EQ(std::error_code(Spec_Violated), CommandError::Spec_Violated);
    EXPECT_EQ(std::error_code(Protobuf_Parsing_Failed), CommandError::Protobuf_Parsing_Failed);
    EXPECT_EQ(std::error_code(File_Read_Failed), CommandError::File_Operation_Failed);
    EXPECT_EQ(std::error_code(File_Write_Failed), CommandError::File_Operation_Failed);
    EXPECT_EQ(std::error_code(Invalid_Project_Dir), CommandError::Invalid_Argument);
}

TEST(GenDocCommandTest, Help_Is_Defined_For_The_Command)
{
    HelpCommand helpCmd({CommandId::GenDoc});
    std::ostringstream out, err;

    EXPECT_NO_THROW(helpCmd.execute(out, err));
    EXPECT_TRUE(IsHelpMessage(out.str(), CommandId::GenDoc));
    EXPECT_TRUE(err.str().empty());
}

TEST(GenDocCommandTest, Command_Succeeds_For_Valid_Project_And_Outputs_Success_Message)
{
    std::ostringstream out, err;
    TmpDir tmp;
    TmpDir outputDir("out");
    CreateMinimalProject(tmp);

    EXPECT_NO_THROW(GenDocCommand({GenDocFormat::Json, "tmp", "out", BUSRPC_TESTS_PROTOBUF_ROOT}).execute(out, err));
    EXPECT_FALSE(out.str().empty());
    EXPECT_TRUE(err.str().empty());
    EXPECT_TRUE(std::filesystem::is_regular_file(std::string("out/") + Json_Doc_File));
}

TEST(GenDocCommandTest, Invalid_Project_Dir_Error_If_Project_Dir_Does_Not_Exist)
{
    std::ostringstream err;
    TmpDir outputDir("out");

    EXPECT_COMMAND_EXCEPTION(
        GenDocCommand({GenDocFormat::Json, "missing_project_dir", "out"}).execute(std::nullopt, err),
        GenDocErrc::Invalid_Project_Dir);
    EXPECT_FALSE(err.str().empty());
    EXPECT_FALSE(std::filesystem::exists(std::string("out/") + Json_Doc_File));
}

TEST(GenDocCommandTest, File_Write_Failed_Error_If_Output_Dir_Does_Not_Exist)
{
    std::ostringstream err;
    TmpDir tmp;
    TmpDir outputDir("out");
    CreateMinimalProject(tmp);

    EXPECT_COMMAND_EXCEPTION(
        GenDocCommand({GenDocFormat::Json, "tmp", "out/nonexistent_dir"}).execute(std::nullopt, err),
        GenDocErrc::File_Write_Failed);
    EXPECT_FALSE(err.str().empty());
}

TEST(GenDocCommandTest, Protobuf_Parsing_Failed_Error_If_Some_File_Is_Not_Parsed)
{
    std::ostringstream err;
    TmpDir tmp;
    TmpDir outputDir("out");
    CreateMinimalProject(tmp);
    tmp.writeFile("invalid.proto", "syntax =");

    EXPECT_COMMAND_EXCEPTION(
        GenDocCommand({GenDocFormat::Json, "tmp", "out", BUSRPC_TESTS_PROTOBUF_ROOT}).execute(std::nullopt, err),
        GenDocErrc::Protobuf_Parsing_Failed);
    EXPECT_FALSE(err.str().empty());
    EXPECT_TRUE(std::filesystem::is_regular_file(std::string("out/") + Json_Doc_File));
}

TEST(GenDocCommandTest, Spec_Violated_Error_If_Spec_Error_Detected)
{
    std::ostringstream err;
    TmpDir tmp;
    TmpDir outputDir("out");
    CreateMinimalProject(tmp);

    std::string invalidType = "syntax = \"proto3\";\n"
                              "package busrpc.aaa;\n"
                              "message MyStruct {}";
    tmp.writeFile("file.proto", invalidType);

    EXPECT_COMMAND_EXCEPTION(
        GenDocCommand({GenDocFormat::Json, "tmp", "out", BUSRPC_TESTS_PROTOBUF_ROOT}).execute(std::nullopt, err),
        GenDocErrc::Spec_Violated);
    EXPECT_FALSE(err.str().empty());
    EXPECT_TRUE(std::filesystem::is_regular_file(std::string("out/") + Json_Doc_File));
}

TEST(GenDocCommandTest, Command_Outputs_Doc_Warnings_But_Still_Indicates_Success)
{
    std::ostringstream out, err;
    TmpDir tmp;
    TmpDir outputDir("out");
    CreateMinimalProject(tmp);

    std::string undocumentedStruct = "syntax = \"proto3\";\n"
                                     "package busrpc;\n"
                                     "message MyStruct {}";
    tmp.writeFile("file.proto", undocumentedStruct);

    EXPECT_NO_THROW(GenDocCommand({GenDocFormat::Json, "tmp", "out", BUSRPC_TESTS_PROTOBUF_ROOT}).execute(out, err));
    EXPECT_FALSE(out.str().empty());
    EXPECT_FALSE(err.str().empty());
    EXPECT_TRUE(std::filesystem::is_regular_file(std::string("out/") + Json_Doc_File));
}

TEST(GenDocCommandTest, Command_Ignores_Spec_Warnings)
{
    std::ostringstream out, err;
    TmpDir tmp;
    TmpDir outputDir("out");
    CreateMinimalProject(tmp);
    tmp.createDir("unexpected_dir");

    EXPECT_NO_THROW(GenDocCommand({GenDocFormat::Json, "tmp", "out", BUSRPC_TESTS_PROTOBUF_ROOT}).execute(out, err));
    EXPECT_FALSE(out.str().empty());
    EXPECT_TRUE(err.str().empty());
    EXPECT_TRUE(std::filesystem::is_regular_file(std::string("out/") + Json_Doc_File));
}

TEST(GenDocCommandTest, Command_Ignores_Style_Warnings)
{
    std::ostringstream out, err;
    TmpDir tmp;
    TmpDir outputDir("out");
    CreateMinimalProject(tmp);

    std::string structWithNonconformingName = "syntax = \"proto3\";\n"
                                              "package busrpc;\n"
                                              "// Structure.\n"
                                              "message my_struct {}";
    tmp.writeFile("file.proto", structWithNonconformingName);

    EXPECT_NO_THROW(GenDocCommand({GenDocFormat::Json, "tmp", "out", BUSRPC_TESTS_PROTOBUF_ROOT}).execute(out, err));
    EXPECT_FALSE(out.str().empty());
    EXPECT_TRUE(err.str().empty());
    EXPECT_TRUE(std::filesystem::is_regular_file(std::string("out/") + Json_Doc_File));
}

TEST(GenDocCommandTest, App_Runs_Command_If_Command_Name_Is_Specified_As_Subcommand)
{
    std::ostringstream out, err;
    TmpDir tmp;
    TmpDir outputDir("out");
    CreateMinimalProject(tmp);

    CLI::App app;
    InitApp(app, out, err);

    int argc = 10;
    const char* argv[] = {"busrpc",
                          GetCommandName(CommandId::GenDoc),
                          "-r",
                          "tmp",
                          "-p",
                          BUSRPC_TESTS_PROTOBUF_ROOT,
                          "-d",
                          "out",
                          "--format",
                          "json"};

    EXPECT_NO_THROW(app.parse(argc, argv));
    EXPECT_FALSE(out.str().empty());
    EXPECT_TRUE(err.str().empty());
    EXPECT_TRUE(std::filesystem::is_regular_file(std::string("out/") + Json_Doc_File));
}
}} // namespace busrpc::test
