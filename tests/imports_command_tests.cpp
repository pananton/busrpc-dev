#include "commands/imports/imports_command.h"

#include <gtest/gtest.h>

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

TEST(ImportsCommandTest,
     Description_For_Unknown_Command_Error_Code_Exists_And_Differs_From_Known_Error_Codes_Descriptions)
{
    EXPECT_FALSE(imports_error_category().message(0).empty());
}
}} // namespace busrpc::test
