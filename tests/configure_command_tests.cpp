#include "commands/configure/configure_command.h"

#include <gtest/gtest.h>

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
    EXPECT_NE(configure_error_category().message(static_cast<int>(ConfigureErrc::Non_Existent_Root_Error)),
              configure_error_category().message(0));
    EXPECT_NE(configure_error_category().message(static_cast<int>(ConfigureErrc::File_Not_Found)),
              configure_error_category().message(0));
    EXPECT_NE(configure_error_category().message(static_cast<int>(ConfigureErrc::Protobuf_Error)),
              configure_error_category().message(0));
    EXPECT_NE(configure_error_category().message(static_cast<int>(ConfigureErrc::Create_Output_Dir_Error)),
              configure_error_category().message(0));
    EXPECT_NE(configure_error_category().message(static_cast<int>(ConfigureErrc::File_Read_Error)),
              configure_error_category().message(0));
    EXPECT_NE(configure_error_category().message(static_cast<int>(ConfigureErrc::File_Write_Error)),
              configure_error_category().message(0));
}

TEST(ConfigureCommandTest, Error_Codes_Are_Mapped_To_Appropriate_Error_Conditions)
{
    EXPECT_EQ(std::error_code(ConfigureErrc::Non_Existent_Root_Error), CommandError::Argument_Error);
    EXPECT_EQ(std::error_code(ConfigureErrc::File_Not_Found), CommandError::Argument_Error);
    EXPECT_EQ(std::error_code(ConfigureErrc::Protobuf_Error), CommandError::Protobuf_Error);
    EXPECT_EQ(std::error_code(ConfigureErrc::Create_Output_Dir_Error), CommandError::File_Access_Error);
    EXPECT_EQ(std::error_code(ConfigureErrc::File_Read_Error), CommandError::File_Access_Error);
    EXPECT_EQ(std::error_code(ConfigureErrc::File_Write_Error), CommandError::File_Access_Error);
}
}} // namespace busrpc::test
