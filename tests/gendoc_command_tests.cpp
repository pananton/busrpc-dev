#include "commands/gendoc/gendoc_command.h"
#include "commands/help/help_command.h"
#include "app.h"
#include "utils.h"

#include <CLI/CLI.hpp>
#include <gtest/gtest.h>

#include <sstream>

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

TEST(GenDocCommandTest, Description_For_Unknown_Command_Error_Code_Is_Not_Empty)
{
    EXPECT_FALSE(gendoc_error_category().message(0).empty());
}

TEST(GenDocCommandTest, Description_For_Unknown_Command_Error_Code_Differs_From_Known_Error_Codes_Descriptions)
{
    EXPECT_NE(gendoc_error_category().message(static_cast<int>(GenDocErrc::Non_Existent_Root_Error)),
              gendoc_error_category().message(0));
    EXPECT_NE(gendoc_error_category().message(static_cast<int>(GenDocErrc::Specification_Error)),
              gendoc_error_category().message(0));
    EXPECT_NE(gendoc_error_category().message(static_cast<int>(GenDocErrc::Protobuf_Error)),
              gendoc_error_category().message(0));
    EXPECT_NE(gendoc_error_category().message(static_cast<int>(GenDocErrc::Create_Output_Dir_Error)),
              gendoc_error_category().message(0));
    EXPECT_NE(gendoc_error_category().message(static_cast<int>(GenDocErrc::File_Read_Error)),
              gendoc_error_category().message(0));
    EXPECT_NE(gendoc_error_category().message(static_cast<int>(GenDocErrc::File_Write_Error)),
              gendoc_error_category().message(0));
}

TEST(GenDocCommandTest, Error_Codes_Are_Mapped_To_Appropriate_Error_Conditions)
{
    EXPECT_EQ(std::error_code(GenDocErrc::Non_Existent_Root_Error), CommandError::Argument_Error);
    EXPECT_EQ(std::error_code(GenDocErrc::Specification_Error), CommandError::Logic_Error);
    EXPECT_EQ(std::error_code(GenDocErrc::Protobuf_Error), CommandError::Protobuf_Error);
    EXPECT_EQ(std::error_code(GenDocErrc::Create_Output_Dir_Error), CommandError::File_Access_Error);
    EXPECT_EQ(std::error_code(GenDocErrc::File_Read_Error), CommandError::File_Access_Error);
    EXPECT_EQ(std::error_code(GenDocErrc::File_Write_Error), CommandError::File_Access_Error);
}

TEST(GenDocCommandTest, Help_Is_Defined_For_The_Command)
{
    HelpCommand helpCmd({CommandId::GenDoc});
    std::ostringstream out, err;

    EXPECT_NO_THROW(helpCmd.execute(out, err));
    EXPECT_TRUE(IsHelpMessage(out.str(), CommandId::GenDoc));
    EXPECT_TRUE(err.str().empty());
}
}} // namespace busrpc::test
