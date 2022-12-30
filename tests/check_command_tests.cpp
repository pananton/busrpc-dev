#include "app.h"
#include "commands/check/check_command.h"
#include "commands/help/help_command.h"
#include "tests_configure.h"
#include "utils/common.h"
#include "utils/project_utils.h"

#include <CLI/CLI.hpp>
#include <gtest/gtest.h>

#include <sstream>

namespace busrpc { namespace test {

TEST(CheckCommandTest, Command_Name_And_Id_Are_Mapped_To_Each_Other)
{
    EXPECT_EQ(CommandId::Check, GetCommandId(GetCommandName(CommandId::Check)));
    EXPECT_EQ(CheckCommand::Id, CommandId::Check);
    EXPECT_STREQ(CheckCommand::Name, GetCommandName(CommandId::Check));
}

TEST(CheckCommandTest, Command_Error_Category_Name_Matches_Command_Name)
{
    EXPECT_STREQ(check_error_category().name(), GetCommandName(CommandId::Check));
}

TEST(CheckCommandTest, Description_For_Unknown_Command_Error_Code_Is_Not_Empty)
{
    EXPECT_FALSE(check_error_category().message(0).empty());
}

TEST(CheckCommandTest, Description_For_Unknown_Command_Error_Code_Differs_From_Known_Error_Codes_Descriptions)
{
    EXPECT_NE(check_error_category().message(static_cast<int>(CheckErrc::Style_Violated)),
              check_error_category().message(0));
    EXPECT_NE(check_error_category().message(static_cast<int>(CheckErrc::Doc_Rule_Violated)),
              check_error_category().message(0));
    EXPECT_NE(check_error_category().message(static_cast<int>(CheckErrc::Spec_Violated)),
              check_error_category().message(0));
    EXPECT_NE(check_error_category().message(static_cast<int>(CheckErrc::Protobuf_Parsing_Failed)),
              check_error_category().message(0));
    EXPECT_NE(check_error_category().message(static_cast<int>(CheckErrc::File_Read_Failed)),
              check_error_category().message(0));
    EXPECT_NE(check_error_category().message(static_cast<int>(CheckErrc::Invalid_Project_Dir)),
              check_error_category().message(0));
}

TEST(CheckCommandTest, Error_Codes_Are_Mapped_To_Appropriate_Error_Conditions)
{
    EXPECT_EQ(std::error_code(CheckErrc::Style_Violated), CommandError::Spec_Violated);
    EXPECT_EQ(std::error_code(CheckErrc::Doc_Rule_Violated), CommandError::Spec_Violated);
    EXPECT_EQ(std::error_code(CheckErrc::Spec_Violated), CommandError::Spec_Violated);
    EXPECT_EQ(std::error_code(CheckErrc::Protobuf_Parsing_Failed), CommandError::Protobuf_Parsing_Failed);
    EXPECT_EQ(std::error_code(CheckErrc::File_Read_Failed), CommandError::File_Operation_Failed);
    EXPECT_EQ(std::error_code(CheckErrc::Invalid_Project_Dir), CommandError::Invalid_Argument);
}

TEST(CheckCommandTest, Help_Is_Defined_For_The_Command)
{
    HelpCommand helpCmd({CommandId::Check});
    std::ostringstream out, err;

    EXPECT_NO_THROW(helpCmd.execute(out, err));
    EXPECT_TRUE(IsHelpMessage(out.str(), CommandId::Check));
    EXPECT_TRUE(err.str().empty());
}

TEST(CheckCommandTest, Command_Succeeds_For_Valid_Project_And_Outputs_Success_Message)
{
    std::ostringstream out, err;
    TmpDir tmp;
    CreateMinimalProject(tmp);

    EXPECT_NO_THROW(CheckCommand({"tmp", BUSRPC_TESTS_PROTOBUF_ROOT}).execute(out, err));
    EXPECT_FALSE(out.str().empty());
    EXPECT_TRUE(err.str().empty());
}

TEST(CheckCommandTest, Invalid_Project_Dir_If_Project_Dir_Does_Not_Exist)
{
    std::ostringstream err;

    EXPECT_COMMAND_EXCEPTION(CheckCommand({"missing_project_dir"}).execute(std::nullopt, err),
                             CheckErrc::Invalid_Project_Dir);
    EXPECT_FALSE(err.str().empty());
}

TEST(CheckCommandTest, Invalid_Project_Dir_If_Project_Dir_Does_Not_Represent_Valid_Busrpc_Project_Dir)
{
    std::ostringstream err;
    TmpDir tmp;

    EXPECT_COMMAND_EXCEPTION(CheckCommand({"tmp", BUSRPC_TESTS_PROTOBUF_ROOT}).execute(std::nullopt, err),
                             CheckErrc::Invalid_Project_Dir);
    EXPECT_FALSE(err.str().empty());
}

TEST(CheckCommandTest, Protobuf_Parsing_Failed_Error_If_Some_File_Is_Not_Parsed)
{
    std::ostringstream err;
    TmpDir tmp;
    CreateMinimalProject(tmp);
    tmp.writeFile("invalid.proto", "syntax =");

    EXPECT_COMMAND_EXCEPTION(CheckCommand({"tmp", BUSRPC_TESTS_PROTOBUF_ROOT}).execute(std::nullopt, err),
                             CheckErrc::Protobuf_Parsing_Failed);
    EXPECT_FALSE(err.str().empty());
}

TEST(CheckCommandTest, Spec_Violated_Error_If_Spec_Error_Detected)
{
    std::ostringstream err;
    TmpDir tmp;
    CreateMinimalProject(tmp);

    std::string invalidType = "syntax = \"proto3\";\n"
                              "package busrpc.aaa;\n"
                              "message MyStruct {}";
    tmp.writeFile("file.proto", invalidType);

    EXPECT_COMMAND_EXCEPTION(CheckCommand({"tmp", BUSRPC_TESTS_PROTOBUF_ROOT}).execute(std::nullopt, err),
                             CheckErrc::Spec_Violated);
    EXPECT_FALSE(err.str().empty());
}

TEST(CheckCommandTest, Command_Succeeds_If_Spec_Warn_Detected_And_Warnings_Are_Not_Errors)
{
    std::ostringstream out, err;
    TmpDir tmp;
    CreateMinimalProject(tmp);
    tmp.createDir("unexpected_dir");

    EXPECT_NO_THROW(CheckCommand({"tmp", BUSRPC_TESTS_PROTOBUF_ROOT}).execute(out, err));
    EXPECT_FALSE(out.str().empty());
    EXPECT_FALSE(err.str().empty());
}

TEST(CheckCommandTest, Spec_Violated_Error_If_Spec_Warn_Detected_And_Warnings_Are_Errors)
{
    std::ostringstream err;
    TmpDir tmp;
    CreateMinimalProject(tmp);
    tmp.createDir("unexpected_dir");

    EXPECT_COMMAND_EXCEPTION(
        CheckCommand({"tmp", BUSRPC_TESTS_PROTOBUF_ROOT, false, false, false, true}).execute(std::nullopt, err),
        CheckErrc::Spec_Violated);
    EXPECT_FALSE(err.str().empty());
}

TEST(CheckCommandTest, Command_Succeeds_If_Spec_Warn_Detected_Warnings_Are_Errors_But_Spec_Warnings_Are_Ignored)
{
    std::ostringstream out, err;
    TmpDir tmp;
    CreateMinimalProject(tmp);
    tmp.createDir("unexpected_dir");

    EXPECT_NO_THROW(CheckCommand({"tmp", BUSRPC_TESTS_PROTOBUF_ROOT, true, false, false, true}).execute(out, err));
    EXPECT_FALSE(out.str().empty());
    EXPECT_TRUE(err.str().empty());
}

TEST(CheckCommandTest, Command_Succeeds_If_Doc_Warn_Detected_And_Warnings_Are_Not_Errors)
{
    std::ostringstream out, err;
    TmpDir tmp;
    CreateMinimalProject(tmp);

    std::string undocumentedStruct = "syntax = \"proto3\";\n"
                                     "package busrpc;\n"
                                     "message MyStruct {}";
    tmp.writeFile("file.proto", undocumentedStruct);

    EXPECT_NO_THROW(CheckCommand({"tmp", BUSRPC_TESTS_PROTOBUF_ROOT}).execute(out, err));
    EXPECT_FALSE(out.str().empty());
    EXPECT_FALSE(err.str().empty());
}

TEST(CheckCommandTest, Doc_Rule_Violated_Error_If_Doc_Warn_Detected_And_Warnings_Are_Errors)
{
    std::ostringstream err;
    TmpDir tmp;
    CreateMinimalProject(tmp);

    std::string undocumentedStruct = "syntax = \"proto3\";\n"
                                     "package busrpc;\n"
                                     "message MyStruct {}";
    tmp.writeFile("file.proto", undocumentedStruct);

    EXPECT_COMMAND_EXCEPTION(
        CheckCommand({"tmp", BUSRPC_TESTS_PROTOBUF_ROOT, false, false, false, true}).execute(std::nullopt, err),
        CheckErrc::Doc_Rule_Violated);
    EXPECT_FALSE(err.str().empty());
}

TEST(CheckCommandTest, Command_Succeeds_If_Doc_Warn_Detected_Warnings_Are_Errors_But_Doc_Warnings_Are_Ignored)
{
    std::ostringstream out, err;
    TmpDir tmp;
    CreateMinimalProject(tmp);

    std::string undocumentedStruct = "syntax = \"proto3\";\n"
                                     "package busrpc;\n"
                                     "message MyStruct {}";
    tmp.writeFile("file.proto", undocumentedStruct);

    EXPECT_NO_THROW(CheckCommand({"tmp", BUSRPC_TESTS_PROTOBUF_ROOT, false, true, false, true}).execute(out, err));
    EXPECT_FALSE(out.str().empty());
    EXPECT_TRUE(err.str().empty());
}

TEST(CheckCommandTest, Command_Succeeds_If_Style_Warn_Detected_And_Warnings_Are_Not_Errors)
{
    std::ostringstream out, err;
    TmpDir tmp;
    CreateMinimalProject(tmp);

    std::string structWithNonconformingName = "syntax = \"proto3\";\n"
                                              "package busrpc;\n"
                                              "// Structure.\n"
                                              "message my_struct {}";
    tmp.writeFile("file.proto", structWithNonconformingName);

    EXPECT_NO_THROW(CheckCommand({"tmp", BUSRPC_TESTS_PROTOBUF_ROOT}).execute(out, err));
    EXPECT_FALSE(out.str().empty());
    EXPECT_FALSE(err.str().empty());
}

TEST(CheckCommandTest, Style_Violated_Error_If_Style_Warn_Detected_And_Warnings_Are_Errors)
{
    std::ostringstream err;
    TmpDir tmp;
    CreateMinimalProject(tmp);

    std::string structWithNonconformingName = "syntax = \"proto3\";\n"
                                              "package busrpc;\n"
                                              "// Structure.\n"
                                              "message my_struct {}";
    tmp.writeFile("file.proto", structWithNonconformingName);

    EXPECT_COMMAND_EXCEPTION(
        CheckCommand({"tmp", BUSRPC_TESTS_PROTOBUF_ROOT, false, false, false, true}).execute(std::nullopt, err),
        CheckErrc::Style_Violated);
    EXPECT_FALSE(err.str().empty());
}

TEST(CheckCommandTest, Command_Succeeds_If_Style_Warn_Detected_Warnings_Are_Errors_But_Style_Warnings_Are_Ignored)
{
    std::ostringstream out, err;
    TmpDir tmp;
    CreateMinimalProject(tmp);

    std::string structWithNonconformingName = "syntax = \"proto3\";\n"
                                              "package busrpc;\n"
                                              "// Structure.\n"
                                              "message my_struct {}";
    tmp.writeFile("file.proto", structWithNonconformingName);

    EXPECT_NO_THROW(CheckCommand({"tmp", BUSRPC_TESTS_PROTOBUF_ROOT, false, false, true, true}).execute(out, err));
    EXPECT_FALSE(out.str().empty());
    EXPECT_TRUE(err.str().empty());
}

TEST(CheckCommandTest, App_Runs_Command_If_Command_Name_Is_Specified_As_Subcommand)
{
    std::ostringstream out, err;
    TmpDir tmp;
    CreateMinimalProject(tmp);

    std::string testStruct = "syntax = \"proto3\";\n"
                             "package busrpc;\n"
                             "message my_struct {}";

    tmp.createDir("unexpected_dir");         // specification warning
    tmp.writeFile("file.proto", testStruct); // documentation and style warning

    CLI::App app;
    InitApp(app, out, err);

    int argc = 10;
    const char* argv[] = {"busrpc",
                          GetCommandName(CommandId::Check),
                          "-r",
                          "tmp",
                          "-p",
                          BUSRPC_TESTS_PROTOBUF_ROOT,
                          "--ignore-spec",
                          "--ignore-doc",
                          "--ignore-style",
                          "-w"};

    EXPECT_NO_THROW(app.parse(argc, argv));
    EXPECT_FALSE(out.str().empty());
    EXPECT_TRUE(err.str().empty());
}
}} // namespace busrpc::test
