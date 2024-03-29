#include "app.h"
#include "commands/help/help_command.h"
#include "commands/imports/imports_command.h"
#include "tests_configure.h"
#include "utils.h"
#include "utils/common.h"
#include "utils/file_utils.h"

#include <CLI/CLI.hpp>
#include <gtest/gtest.h>

#include <sstream>

namespace busrpc { namespace test {

TEST(ImportsCommandTest, Command_Name_And_Id_Are_Mapped_To_Each_Other)
{
    EXPECT_EQ(CommandId::Imports, GetCommandId(GetCommandName(CommandId::Imports)));
    EXPECT_EQ(ImportsCommand::Id, CommandId::Imports);
    EXPECT_STREQ(ImportsCommand::Name, GetCommandName(CommandId::Imports));
}

TEST(ImportsCommandTest, Command_Error_Category_Name_Matches_Command_Name)
{
    EXPECT_STREQ(imports_error_category().name(), GetCommandName(CommandId::Imports));
}

TEST(ImportsCommandTest, Description_For_Unknown_Command_Error_Code_Is_Not_Empty)
{
    EXPECT_FALSE(imports_error_category().message(0).empty());
}

TEST(ImportsCommandTest, Description_For_Unknown_Command_Error_Code_Differs_From_Known_Error_Codes_Descriptions)
{
    EXPECT_NE(imports_error_category().message(static_cast<int>(ImportsErrc::Protobuf_Parsing_Failed)),
              imports_error_category().message(0));
    EXPECT_NE(imports_error_category().message(static_cast<int>(ImportsErrc::File_Read_Failed)),
              imports_error_category().message(0));
    EXPECT_NE(imports_error_category().message(static_cast<int>(ImportsErrc::File_Not_Found)),
              imports_error_category().message(0));
    EXPECT_NE(imports_error_category().message(static_cast<int>(ImportsErrc::Invalid_Project_Dir)),
              imports_error_category().message(0));
}

TEST(ImportsCommandTest, Error_Codes_Are_Mapped_To_Appropriate_Error_Conditions)
{
    EXPECT_EQ(std::error_code(ImportsErrc::Protobuf_Parsing_Failed), CommandError::Protobuf_Parsing_Failed);
    EXPECT_EQ(std::error_code(ImportsErrc::File_Read_Failed), CommandError::File_Operation_Failed);
    EXPECT_EQ(std::error_code(ImportsErrc::File_Not_Found), CommandError::Invalid_Argument);
    EXPECT_EQ(std::error_code(ImportsErrc::Invalid_Project_Dir), CommandError::Invalid_Argument);
}

TEST(ImportsCommandTest, Help_Is_Defined_For_The_Command)
{
    HelpCommand helpCmd({CommandId::Imports});
    std::ostringstream out, err;

    EXPECT_NO_THROW(helpCmd.execute(&out, &err));
    EXPECT_TRUE(IsHelpMessage(out.str(), CommandId::Imports));
    EXPECT_TRUE(err.str().empty());
}

TEST(ImportsCommandTest, Command_Succeeds_If_Invoked_Wo_Files_And_Valid_Project_Dir)
{
    EXPECT_NO_THROW(ImportsCommand({}).execute());
}

TEST(ImportsCommandTest, Command_Outputs_Nothing_If_Invoked_Wo_Files)
{
    std::ostringstream out, err;

    EXPECT_NO_THROW(ImportsCommand({}).execute(&out, &err));
    EXPECT_TRUE(out.str().empty());
    EXPECT_TRUE(err.str().empty());
}

TEST(ImportsCommandTest, Invalid_Project_Dir_Error_If_Project_Dir_Does_Not_Exist)
{
    std::ostringstream err;

    EXPECT_COMMAND_EXCEPTION(ImportsCommand({{"missing_file.proto"}, "missing_project_dir"}).execute(nullptr, &err),
                             ImportsErrc::Invalid_Project_Dir);
    EXPECT_FALSE(err.str().empty());
}

TEST(ImportsCommandTest, Invalid_Project_Dir_Error_If_Invoked_Wo_Files_And_Nonexistent_Project_Dir)
{
    std::ostringstream err;

    EXPECT_COMMAND_EXCEPTION(ImportsCommand({{}, "missing_project_dir"}).execute(nullptr, &err),
                             ImportsErrc::Invalid_Project_Dir);
    EXPECT_FALSE(err.str().empty());
}

TEST(ImportsCommandTest, File_Not_Found_Error_If_Some_File_Does_Not_Exist)
{
    std::ostringstream err;
    TmpDir tmp;
    std::string file = "syntax = \"proto3\";"
                       "package test;";
    tmp.writeFile("file.proto", file);

    EXPECT_COMMAND_EXCEPTION(ImportsCommand({{"file.proto", "missing_file.proto"}, "tmp"}).execute(nullptr, &err),
                             ImportsErrc::File_Not_Found);
    EXPECT_FALSE(err.str().empty());
}

TEST(ImportsCommandTest, File_Not_Found_Error_If_Some_File_Is_Outside_The_Project_Dir)
{
    std::ostringstream err;
    TmpDir tmp;
    std::string file = "syntax = \"proto3\";"
                       "package test.subdir;";
    tmp.writeFile("subdir/file.proto", file);
    tmp.writeFile("external_file.proto", file);

    EXPECT_COMMAND_EXCEPTION(
        ImportsCommand({{"file.proto", "external_file.proto"}, "tmp/subdir"}).execute(nullptr, &err),
        ImportsErrc::File_Not_Found);
    EXPECT_FALSE(err.str().empty());
}

TEST(ImportsCommandTest, Protobuf_Parsing_Failed_Error_If_Some_File_Is_Not_Parsed)
{
    std::ostringstream err;
    TmpDir tmp;
    std::string valid_file = "syntax = \"proto3\";"
                             "package test;";
    std::string invalid_file = "syntax = \"proto3\";"
                               "package test;"
                               "import \"valid_file.proto\";"
                               "message Msg { ";

    tmp.writeFile("valid.proto", valid_file);
    tmp.writeFile("invalid.proto", invalid_file);

    EXPECT_COMMAND_EXCEPTION(ImportsCommand({{"valid.proto", "invalid.proto"}, "tmp"}).execute(nullptr, &err),
                             ImportsErrc::Protobuf_Parsing_Failed);
    EXPECT_FALSE(err.str().empty());
}

TEST(ImportsCommandTest, Command_Ouputs_Files_Only_Once_In_Desc_Order_Of_Their_Nesting_Level)
{
    TmpDir tmp;

    std::string file1 = "syntax = \"proto3\";"
                        "package test;";

    std::string file2 = "syntax = \"proto3\";"
                        "package test.aaa;";

    std::string file3 = "syntax = \"proto3\";"
                        "package test.aaa;"
                        "import \"file1.proto\";"
                        "import \"aaa/bbb/file4.proto\";";

    std::string file4 = "syntax = \"proto3\";"
                        "package test.aaa.bbb;"
                        "import \"zzz/file5.proto\";";

    std::string file5 = "syntax = \"proto3\";"
                        "package test.zzz;"
                        "import \"aaa/file2.proto\";";

    tmp.writeFile("file1.proto", file1);
    tmp.writeFile("aaa/file2.proto", file2);
    tmp.writeFile("aaa/file3.proto", file3);
    tmp.writeFile("aaa/bbb/file4.proto", file4);
    tmp.writeFile("zzz/file5.proto", file5);

    std::vector<std::string> files;
    files.push_back("zzz/file5.proto");
    files.push_back("aaa/111/222/../.././file3.proto");
    files.push_back("file1.proto");
    files.push_back(std::filesystem::absolute(tmp.path()).string() + "/aaa/111/../bbb/./222/../file4.proto");

    std::ostringstream out, err;
    std::vector<std::string> output;
    ImportsCommand cmd({std::move(files), "tmp"});

    EXPECT_NO_THROW(cmd.execute(&out, &err));
    EXPECT_TRUE(err.str().empty());

    output = SplitString(out.str());

    ASSERT_EQ(output.size(), 5);
    EXPECT_EQ(output[0], "aaa/bbb/file4.proto");
    EXPECT_EQ(output[1], "aaa/file2.proto");
    EXPECT_EQ(output[2], "aaa/file3.proto");
    EXPECT_EQ(output[3], "file1.proto");
    EXPECT_EQ(output[4], "zzz/file5.proto");
}

TEST(ImportsCommandTest, Command_Succeeds_If_Same_File_Specified_Multiple_Times)
{
    std::ostringstream out, err;
    TmpDir tmp;

    std::string file1 = "syntax = \"proto3\";"
                        "package test;";
    std::string file2 = "syntax = \"proto3\";"
                        "package test;"
                        "import \"file1.proto\";";

    tmp.writeFile("file1.proto", file1);
    tmp.writeFile("file2.proto", file2);

    EXPECT_NO_THROW(
        ImportsCommand({{"file1.proto", "file2.proto", "file1.proto", "file2.proto"}, "tmp"}).execute(&out, &err));
    EXPECT_TRUE(err.str().empty());

    auto output = SplitString(out.str());

    ASSERT_EQ(output.size(), 2);
    EXPECT_EQ(output[0], "file1.proto");
    EXPECT_EQ(output[1], "file2.proto");
}

TEST(ImportsCommandTest, Command_Ignores_Imported_Builtin_Protobuf_Files)
{
    std::ostringstream out, err;
    TmpDir tmp;

    std::string file1 = "syntax = \"proto3\";"
                        "package test;"
                        "import \"google/protobuf/descriptor.proto\";";
    std::string file2 = "syntax = \"proto3\";"
                        "package test;"
                        "import \"file1.proto\";";

    tmp.writeFile("file1.proto", file1);
    tmp.writeFile("file2.proto", file2);

    EXPECT_NO_THROW(ImportsCommand({{"file2.proto"}, "tmp", BUSRPC_TESTS_PROTOBUF_ROOT}).execute(&out, &err));
    EXPECT_TRUE(err.str().empty());

    auto output = SplitString(out.str());

    ASSERT_EQ(output.size(), 2);
    EXPECT_EQ(output[0], "file1.proto");
    EXPECT_EQ(output[1], "file2.proto");
}

TEST(ImportsCommandTest, Command_Does_Not_Output_Original_Files_If_Only_Deps_Flag_Is_Set)
{
    std::ostringstream out, err;
    TmpDir tmp;

    std::string file1 = "syntax = \"proto3\";"
                        "package test;";
    std::string file2 = "syntax = \"proto3\";"
                        "package test;"
                        "import \"file1.proto\";";
    std::string file3 = "syntax = \"proto3\";"
                        "package test;"
                        "import \"file2.proto\";";
    std::string file4 = "syntax = \"proto3\";"
                        "package test;"
                        "import \"file2.proto\";";

    tmp.writeFile("file1.proto", file1);
    tmp.writeFile("file2.proto", file2);
    tmp.writeFile("file3.proto", file3);
    tmp.writeFile("file4.proto", file4);

    EXPECT_NO_THROW(ImportsCommand({{"file3.proto", "file2.proto", "file4.proto", "file3.proto"}, "tmp", "", true})
                        .execute(&out, &err));
    EXPECT_TRUE(err.str().empty());

    auto output = SplitString(out.str());

    ASSERT_EQ(output.size(), 1);
    EXPECT_EQ(output[0], "file1.proto");
}

TEST(ImportsCommandTest, Command_Tries_To_Proceed_After_Error)
{
    std::ostringstream out, err;
    TmpDir tmp;

    std::string file1 = "syntax = \"proto3\";"
                        "package test;";
    std::string file2 = "syntax = \"proto3\";"
                        "package test;"
                        "import \"file1.proto\";";
    std::string invalidFile = "syntax = ";

    tmp.writeFile("file1.proto", file1);
    tmp.writeFile("file2.proto", file2);
    tmp.writeFile("invalid_file.proto", invalidFile);

    EXPECT_COMMAND_EXCEPTION(
        ImportsCommand({{"missing_file.proto", "invalid_file.proto", "file2.proto"}, "tmp"}).execute(&out, &err),
        ImportsErrc::File_Not_Found);
    EXPECT_FALSE(err.str().empty());

    auto output = SplitString(out.str());

    ASSERT_EQ(output.size(), 2);
    EXPECT_EQ(output[0], "file1.proto");
    EXPECT_EQ(output[1], "file2.proto");
}

TEST(ImportsCommandTest, App_Runs_Command_If_Command_Name_Is_Specified_As_Subcommand)
{
    std::ostringstream out, err;
    TmpDir tmp;

    std::string file1 = "syntax = \"proto3\";"
                        "package test;";
    std::string file2 = "syntax = \"proto3\";"
                        "package test;"
                        "import \"file1.proto\";";

    tmp.writeFile("file1.proto", file1);
    tmp.writeFile("file2.proto", file2);

    CLI::App app;
    InitApp(app, out, err);

    int argc = 8;
    const char* argv[] = {"busrpc",
                          GetCommandName(CommandId::Imports),
                          "-r",
                          "tmp",
                          "-p",
                          BUSRPC_TESTS_PROTOBUF_ROOT,
                          "file1.proto",
                          "file2.proto"};

    EXPECT_NO_THROW(app.parse(argc, argv));
    EXPECT_TRUE(err.str().empty());

    auto output = SplitString(out.str());

    ASSERT_EQ(output.size(), 2);
    EXPECT_EQ(output[0], "file1.proto");
    EXPECT_EQ(output[1], "file2.proto");
}
}} // namespace busrpc::test
