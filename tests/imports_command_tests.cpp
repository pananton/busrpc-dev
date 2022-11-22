#include "commands/imports/imports_command.h"
#include "commands/help/help_command.h"
#include "app.h"
#include "utils.h"

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
    EXPECT_NE(imports_error_category().message(static_cast<int>(ImportsErrc::Non_Existent_Root_Error)),
              imports_error_category().message(0));
    EXPECT_NE(imports_error_category().message(static_cast<int>(ImportsErrc::File_Not_Found)),
              imports_error_category().message(0));
    EXPECT_NE(imports_error_category().message(static_cast<int>(ImportsErrc::Protobuf_Error)),
              imports_error_category().message(0));
    EXPECT_NE(imports_error_category().message(static_cast<int>(ImportsErrc::File_Read_Error)),
              imports_error_category().message(0));
}

TEST(ImportsCommandTest, Error_Codes_Are_Mapped_To_Appropriate_Error_Conditions)
{
    EXPECT_EQ(std::error_code(ImportsErrc::Non_Existent_Root_Error), CommandError::Argument_Error);
    EXPECT_EQ(std::error_code(ImportsErrc::File_Not_Found), CommandError::Argument_Error);
    EXPECT_EQ(std::error_code(ImportsErrc::Protobuf_Error), CommandError::Protobuf_Error);
    EXPECT_EQ(std::error_code(ImportsErrc::File_Read_Error), CommandError::File_Access_Error);
}

TEST(ImportsCommandTest, Help_Is_Defined_For_The_Command)
{
    HelpCommand helpCmd({CommandId::Imports});
    std::ostringstream out, err;

    EXPECT_NO_THROW(helpCmd.execute(out, err));
    EXPECT_TRUE(IsHelpMessage(out.str(), CommandId::Imports));
    EXPECT_TRUE(err.str().empty());
}
}} // namespace busrpc::test
