#include "commands/version/version_command.h"

#include <gtest/gtest.h>

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

TEST(VersionCommandTest,
     Description_For_Unknown_Command_Error_Code_Exists_And_Differs_From_Known_Error_Codes_Descriptions)
{
    EXPECT_FALSE(version_error_category().message(0).empty());
}
}} // namespace busrpc::test
