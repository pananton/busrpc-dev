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

TEST(ConfigureCommandTest,
     Description_For_Unknown_Command_Error_Code_Exists_And_Differs_From_Known_Error_Codes_Descriptions)
{
    EXPECT_FALSE(configure_error_category().message(0).empty());
    EXPECT_NE(configure_error_category().message(static_cast<int>(ConfigureErrc::Root_Not_Exists)),
              configure_error_category().message(0));
    EXPECT_NE(configure_error_category().message(static_cast<int>(ConfigureErrc::Create_Output_Dir_Error)),
              configure_error_category().message(0));
    EXPECT_NE(configure_error_category().message(static_cast<int>(ConfigureErrc::File_Not_Found)),
              configure_error_category().message(0));
    EXPECT_NE(configure_error_category().message(static_cast<int>(ConfigureErrc::File_Read_Error)),
              configure_error_category().message(0));
    EXPECT_NE(configure_error_category().message(static_cast<int>(ConfigureErrc::Protobuf_Syntax_Error)),
              configure_error_category().message(0));
    EXPECT_NE(configure_error_category().message(static_cast<int>(ConfigureErrc::File_Write_Error)),
              configure_error_category().message(0));
}
}} // namespace busrpc::test
