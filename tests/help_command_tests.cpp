#include "app.h"
#include "commands/help/help_command.h"
#include "utils/common.h"

#include <CLI/CLI.hpp>
#include <gtest/gtest.h>

#include <sstream>

namespace busrpc { namespace test {

TEST(HelpCommandTest, Command_Name_And_Id_Are_Mapped_To_Each_Other)
{
    EXPECT_EQ(CommandId::Help, GetCommandId(GetCommandName(CommandId::Help)));
    EXPECT_EQ(HelpCommand::Id, CommandId::Help);
    EXPECT_STREQ(HelpCommand::Name, GetCommandName(CommandId::Help));
}

TEST(HelpCommandTest, Command_Error_Category_Name_Matches_Command_Name)
{
    EXPECT_STREQ(help_error_category().name(), GetCommandName(CommandId::Help));
}

TEST(HelpCommandTest, Description_For_Unknown_Command_Error_Code_Is_Not_Empty)
{
    EXPECT_FALSE(help_error_category().message(0).empty());
}

TEST(HelpCommandTest, Help_Is_Defined_For_The_Command)
{
    HelpCommand helpCmd({CommandId::Help});
    std::ostringstream out, err;

    EXPECT_NO_THROW(helpCmd.execute(out, err));
    EXPECT_TRUE(IsHelpMessage(out.str(), CommandId::Help));
    EXPECT_TRUE(err.str().empty());
}

TEST(HelpCommandTest, Command_Outputs_Description_Of_All_Commands_If_Invoked_Wo_Args)
{
    HelpCommand helpCmd({});
    std::ostringstream out, err;

    EXPECT_NO_THROW(helpCmd.execute(out, err));
    EXPECT_TRUE(IsHelpMessage(out.str()));
    EXPECT_TRUE(err.str().empty());
}

TEST(HelpCommandTest, App_Runs_Command_If_Command_Name_Is_Specified_As_Subcommand)
{
    std::ostringstream out;
    CLI::App app;
    InitApp(app, out);

    int argc = 3;
    const char* argv[] = {"busrpc", GetCommandName(CommandId::Help), GetCommandName(CommandId::Version)};

    EXPECT_NO_THROW(app.parse(argc - 1, argv));
    EXPECT_TRUE(IsHelpMessage(out.str()));

    out.str("");
    app.clear();

    EXPECT_NO_THROW(app.parse(argc, argv));
    EXPECT_TRUE(IsHelpMessage(out.str(), CommandId::Version));
}
}} // namespace busrpc::test
