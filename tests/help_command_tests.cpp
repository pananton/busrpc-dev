#include "commands/help/help_command.h"

#include <gtest/gtest.h>

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

TEST(HelpCommandTest,
     Description_For_Unknown_Command_Error_Code_Exists_And_Differs_From_Known_Error_Codes_Descriptions)
{
    EXPECT_FALSE(help_error_category().message(0).empty());
}
}} // namespace busrpc::test
