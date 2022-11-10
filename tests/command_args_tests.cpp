#include "commands/command_args.h"

#include <gtest/gtest.h>

#include <filesystem>

TEST(CommandArgsTest, Args_Passed_In_Ctor_Saved_Correctly)
{
    NamedArgs namedArgs = {{"arg1", "value1"}, {"arg2", std::nullopt}};
    PosArgs posArgs = {"value1", "value2"};

    CommandArgs args("cmd", namedArgs, posArgs);

    EXPECT_EQ(args.cmd(), "cmd");
    EXPECT_EQ(args.namedArgs(), namedArgs);
    EXPECT_EQ(args.posArgs(), posArgs);
}

TEST(CommandArgsTest, Command_Name_Parsed_Correctly)
{
    const char* argv[] = {"cmd"};
    CommandArgs args(1, argv);

    EXPECT_EQ(args.cmd(), "cmd");
}

TEST(CommandArgsTest, Named_Args_With_Values_Parsed_Correctly)
{
    const char* argv[] = {"cmd", "-arg1", "value1", "-arg2", "value2"};
    CommandArgs args(5, argv);

    EXPECT_EQ(args.namedArgs().size(), 2);
    ASSERT_EQ(args.namedArgs().count("arg1"), 1);
    ASSERT_EQ(args.namedArgs().count("arg2"), 1);
    ASSERT_TRUE(args.namedArgs().find("arg1")->second);
    ASSERT_TRUE(args.namedArgs().find("arg2")->second);
    EXPECT_EQ(*(args.namedArgs().find("arg1")->second), "value1");
    EXPECT_EQ(*(args.namedArgs().find("arg2")->second), "value2");
}

TEST(CommandArgsTest, Named_Args_Wo_Values_Parsed_Correctly)
{
    const char* argv[] = {"cmd", "-arg1", "-arg2", "-arg3"};
    CommandArgs args(4, argv);

    EXPECT_EQ(args.namedArgs().size(), 3);
    ASSERT_EQ(args.namedArgs().count("arg1"), 1);
    ASSERT_EQ(args.namedArgs().count("arg2"), 1);
    ASSERT_EQ(args.namedArgs().count("arg3"), 1);
    EXPECT_FALSE(args.namedArgs().find("arg1")->second);
    EXPECT_FALSE(args.namedArgs().find("arg2")->second);
    EXPECT_FALSE(args.namedArgs().find("arg3")->second);
}

TEST(CommandArgsTest, Mix_Of_Named_Args_With_Values_And_Wo_Values_Parsed_Correctly)
{
    const char* argv[] = {"cmd", "-arg1", "value1", "-arg2", "-arg3", "value3"};
    CommandArgs args(6, argv);

    EXPECT_EQ(args.namedArgs().size(), 3);
    ASSERT_EQ(args.namedArgs().count("arg1"), 1);
    ASSERT_EQ(args.namedArgs().count("arg2"), 1);
    ASSERT_EQ(args.namedArgs().count("arg3"), 1);
    ASSERT_TRUE(args.namedArgs().find("arg1")->second);
    EXPECT_FALSE(args.namedArgs().find("arg2")->second);
    ASSERT_TRUE(args.namedArgs().find("arg3")->second);
    EXPECT_EQ(*(args.namedArgs().find("arg1")->second), "value1");
    EXPECT_EQ(*(args.namedArgs().find("arg3")->second), "value3");
}

TEST(CommandArgsTest, Root_Named_Arg_Is_Provided_By_Corresponding_Method)
{
    const char* root = "dir1/dir2/dir3";
    CommandArgs args("cmd", {{"arg1", "value1"}, {"r", root}, {"arg2", std::nullopt}});

    EXPECT_EQ(args.root(), std::filesystem::path(root));

    const char* argv[] = {"cmd", "-arg1", "value1", "-r", root, "-arg2"};
    args = CommandArgs(6, argv);

    EXPECT_EQ(args.root(), std::filesystem::path(root));
}

TEST(CommandArgsTest, Root_Named_Arg_Is_Not_Added_To_Named_Args_Container)
{
    CommandArgs args("cmd", {{"arg1", "value1"}, {"r", "dir1/dir2/dir3"}, {"arg2", std::nullopt}});

    EXPECT_EQ(args.namedArgs().size(), 2);
    EXPECT_EQ(args.namedArgs().count("r"), 0);

    const char* argv[] = {"cmd", "-arg1", "value1", "-r", "dir1/dir2/dir3", "-arg2"};
    args = CommandArgs(6, argv);

    EXPECT_EQ(args.namedArgs().size(), 2);
    EXPECT_EQ(args.namedArgs().count("r"), 0);
}

TEST(CommandArgsTest, Default_Value_For_Root_Named_Arg_Is_Working_Dir)
{
    CommandArgs args("cmd");

    EXPECT_EQ(args.root(), std::filesystem::current_path());

    const char* argv[] = {"cmd"};
    args = CommandArgs(1, argv);

    EXPECT_EQ(args.root(), std::filesystem::current_path());
}

TEST(CommandArgsTest, Ctor_Throws_If_Root_Named_Arg_Does_Not_Have_Value)
{
    const char* argv[] = {"cmd", "-r"};

    EXPECT_THROW(CommandArgs("cmd", {{"r", std::nullopt}}), args_error);
    EXPECT_THROW(CommandArgs(2, argv), args_error);
}

TEST(CommandArgsTest, Last_Named_Arg_Value_Is_Saved_If_Arg_Specified_More_Than_Once)
{
    const char* root = "dir1/dir2/dir3";
    const char* argv[] = {
        "cmd", "-r", "old_root/", "-arg1", "old_value1", "-arg2", "old_value2", "-arg2", "-arg1", "value1", "-r", root};
    CommandArgs args(12, argv);

    EXPECT_EQ(args.root(), std::filesystem::path(root));
    EXPECT_EQ(args.namedArgs().size(), 2);
    ASSERT_EQ(args.namedArgs().count("arg1"), 1);
    ASSERT_EQ(args.namedArgs().count("arg2"), 1);
    ASSERT_TRUE(args.namedArgs().find("arg1")->second);
    EXPECT_FALSE(args.namedArgs().find("arg2")->second);
    EXPECT_EQ(*(args.namedArgs().find("arg1")->second), "value1");
}

TEST(CommandArgsTest, Pos_Args_Read_Correctly)
{
    const char* argv[] = {"cmd", "value1", "-value2", "value3"};
    CommandArgs args(4, argv);

    ASSERT_EQ(args.posArgs().size(), 3);
    EXPECT_EQ(args.posArgs()[0], "value1");
    EXPECT_EQ(args.posArgs()[1], "-value2");
    EXPECT_EQ(args.posArgs()[2], "value3");
}

TEST(CommandArgsTest, Double_Hyphen_Separates_Named_Args_From_Pos_Args)
{
    const char* argv[] = {"cmd", "-arg1", "--", "-arg1", "-value1", "-arg2"};
    CommandArgs args(6, argv);

    EXPECT_EQ(args.namedArgs().size(), 1);
    ASSERT_EQ(args.namedArgs().count("arg1"), 1);
    EXPECT_FALSE(args.namedArgs().find("arg1")->second);
    ASSERT_EQ(args.posArgs().size(), 3);
    EXPECT_EQ(args.posArgs()[0], "-arg1");
    EXPECT_EQ(args.posArgs()[1], "-value1");
    EXPECT_EQ(args.posArgs()[2], "-arg2");
}

TEST(CommandArgsTest, Double_Hyphen_Specified_When_No_Pos_Params_Exist_Does_Not_Break_Parsing)
{
    const char* argv[] = {"cmd", "-arg1", "-arg2", "value2", "--"};
    CommandArgs args(5, argv);

    EXPECT_EQ(args.namedArgs().size(), 2);
    ASSERT_EQ(args.namedArgs().count("arg1"), 1);
    ASSERT_EQ(args.namedArgs().count("arg2"), 1);
    EXPECT_FALSE(args.namedArgs().find("arg1")->second);
    ASSERT_TRUE(args.namedArgs().find("arg2")->second);
    EXPECT_EQ(*(args.namedArgs().find("arg2")->second), "value2");
    EXPECT_EQ(args.posArgs().size(), 0);
}

TEST(CommandArgsTest, Double_Hyphen_Specified_When_No_Params_Exist_Does_Not_Break_Parsing)
{
    const char* argv[] = {"cmd", "--"};
    CommandArgs args(2, argv);

    EXPECT_EQ(args.namedArgs().size(), 0);
    EXPECT_EQ(args.posArgs().size(), 0);
}
