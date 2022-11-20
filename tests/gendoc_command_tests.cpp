#include "commands/gendoc/gendoc_command.h"

#include <gtest/gtest.h>

namespace busrpc { namespace test {

TEST(GenDocCommandTest, Command_Name_And_Id_Are_Mapped_To_Each_Other)
{
    EXPECT_EQ(CommandId::GenDoc, GetCommandId(GetCommandName(CommandId::GenDoc)));
    EXPECT_EQ(GenDocCommand::Id, CommandId::GenDoc);
    EXPECT_STREQ(GenDocCommand::Name, GetCommandName(CommandId::GenDoc));
}

TEST(GenDocCommandTest, Command_Error_Category_Name_Matches_Command_Name)
{
    EXPECT_STREQ(gendoc_error_category().name(), GetCommandName(CommandId::GenDoc));
}

TEST(GenDocCommandTest,
     Description_For_Unknown_Command_Error_Code_Exists_And_Differs_From_Known_Error_Codes_Descriptions)
{
    EXPECT_FALSE(gendoc_error_category().message(0).empty());
    EXPECT_NE(gendoc_error_category().message(static_cast<int>(GenDocErrc::Create_Output_Dir_Error)),
              gendoc_error_category().message(0));
    EXPECT_NE(gendoc_error_category().message(static_cast<int>(GenDocErrc::Read_Error)),
              gendoc_error_category().message(0));
    EXPECT_NE(gendoc_error_category().message(static_cast<int>(GenDocErrc::Protobuf_Syntax_Error)),
              gendoc_error_category().message(0));
    EXPECT_NE(gendoc_error_category().message(static_cast<int>(GenDocErrc::Write_Error)),
              gendoc_error_category().message(0));
}
}} // namespace busrpc::test
