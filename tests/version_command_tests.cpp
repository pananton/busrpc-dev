#include "app.h"
#include "commands/help/help_command.h"
#include "commands/version/version_command.h"
#include "configure.h"
#include "utils/common.h"

#include <CLI/CLI.hpp>
#include <gtest/gtest.h>

#include <sstream>

namespace busrpc { namespace test {

TEST(VersionCommandTest, Command_Name_And_Id_Are_Mapped_To_Each_Other)
{
    EXPECT_EQ(CommandId::Version, GetCommandId(GetCommandName(CommandId::Version)));
    EXPECT_EQ(VersionCommand::Id, CommandId::Version);
    EXPECT_STREQ(VersionCommand::Name, GetCommandName(CommandId::Version));
}

TEST(VersionCommandTest, Command_Error_Category_Name_Matches_Command_Name)
{
    EXPECT_STREQ(version_error_category().name(), GetCommandName(CommandId::Version));
}

TEST(VersionCommandTest, Description_For_Unknown_Command_Error_Code_Is_Not_Empty)
{
    EXPECT_FALSE(version_error_category().message(0).empty());
}

TEST(VersionCommandTest, Help_Is_Defined_For_The_Command)
{
    HelpCommand helpCmd({CommandId::Version});
    std::ostringstream out, err;

    EXPECT_NO_THROW(helpCmd.execute(out, err));
    EXPECT_TRUE(IsHelpMessage(out.str(), CommandId::Version));
    EXPECT_TRUE(err.str().empty());
}

TEST(VersionCommandTest, Command_Outputs_Current_Version)
{
    VersionCommand cmd({});
    std::ostringstream out, err;

    EXPECT_NO_THROW(cmd.execute(out, err));
    EXPECT_NE(out.str().find(BUSRPC_VERSION), std::string::npos);
    EXPECT_TRUE(err.str().empty());
}

TEST(VersionCommandTest, App_Runs_Command_If_Command_Name_Is_Specified_As_Subcommand)
{
    std::ostringstream out;
    CLI::App app;
    InitApp(app, out);

    int argc = 2;
    const char* argv[] = {"busrpc", GetCommandName(CommandId::Version)};

    EXPECT_NO_THROW(app.parse(argc, argv));
    EXPECT_NE(out.str().find(BUSRPC_VERSION), std::string::npos);
}
}} // namespace busrpc::test
