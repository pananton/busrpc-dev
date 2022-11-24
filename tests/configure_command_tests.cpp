#include "app.h"
#include "commands/configure/configure_command.h"
#include "commands/help/help_command.h"
#include "utils/common.h"

#include <CLI/CLI.hpp>
#include <gtest/gtest.h>

#include <sstream>

namespace busrpc { namespace test {

TEST(ConfigureCommandTest, Command_Name_And_Id_Are_Mapped_To_Each_Other)
{
    EXPECT_EQ(CommandId::Configure, GetCommandId(GetCommandName(CommandId::Configure)));
    EXPECT_EQ(ConfigureCommand::Id, CommandId::Configure);
    EXPECT_STREQ(ConfigureCommand::Name, GetCommandName(CommandId::Configure));
}

TEST(ConfigureCommandTest, Command_Error_Category_Name_Matches_Command_Name)
{
    EXPECT_STREQ(configure_error_category().name(), GetCommandName(CommandId::Configure));
}

TEST(ConfigureCommandTest, Description_For_Unknown_Command_Error_Code_Is_Not_Empty)
{
    EXPECT_FALSE(configure_error_category().message(0).empty());
}

TEST(ConfigureCommandTest, Description_For_Unknown_Command_Error_Code_Differs_From_Known_Error_Codes_Descriptions)
{
    EXPECT_NE(configure_error_category().message(static_cast<int>(ConfigureErrc::File_Write_Failed)),
              configure_error_category().message(0));
    EXPECT_NE(configure_error_category().message(static_cast<int>(ConfigureErrc::File_Read_Failed)),
              configure_error_category().message(0));
    EXPECT_NE(configure_error_category().message(static_cast<int>(ConfigureErrc::Create_Output_Dir_Failed)),
              configure_error_category().message(0));
    EXPECT_NE(configure_error_category().message(static_cast<int>(ConfigureErrc::Protobuf_Parsing_Failed)),
              configure_error_category().message(0));
    EXPECT_NE(configure_error_category().message(static_cast<int>(ConfigureErrc::File_Not_Found)),
              configure_error_category().message(0));
    EXPECT_NE(configure_error_category().message(static_cast<int>(ConfigureErrc::Root_Does_Not_Exist)),
              configure_error_category().message(0));
}

TEST(ConfigureCommandTest, Error_Codes_Are_Mapped_To_Appropriate_Error_Conditions)
{
    EXPECT_EQ(std::error_code(ConfigureErrc::File_Write_Failed), CommandError::File_Operation_Failed);
    EXPECT_EQ(std::error_code(ConfigureErrc::File_Read_Failed), CommandError::File_Operation_Failed);
    EXPECT_EQ(std::error_code(ConfigureErrc::Create_Output_Dir_Failed), CommandError::File_Operation_Failed);
    EXPECT_EQ(std::error_code(ConfigureErrc::Protobuf_Parsing_Failed), CommandError::Protobuf_Parsing_Failed);
    EXPECT_EQ(std::error_code(ConfigureErrc::File_Not_Found), CommandError::Invalid_Argument);
    EXPECT_EQ(std::error_code(ConfigureErrc::Root_Does_Not_Exist), CommandError::Invalid_Argument);
}

TEST(ConfigureCommandTest, Help_Is_Defined_For_The_Command)
{
    HelpCommand helpCmd({CommandId::Configure});
    std::ostringstream out, err;

    EXPECT_NO_THROW(helpCmd.execute(out, err));
    EXPECT_TRUE(IsHelpMessage(out.str(), CommandId::Configure));
    EXPECT_TRUE(err.str().empty());
}
}} // namespace busrpc::test
