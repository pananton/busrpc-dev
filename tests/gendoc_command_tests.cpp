#include "app.h"
#include "commands/gendoc/gendoc_command.h"
#include "commands/help/help_command.h"
#include "utils/common.h"

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
    using enum GenDocErrc;

    EXPECT_NE(gendoc_error_category().message(static_cast<int>(File_Write_Failed)), gendoc_error_category().message(0));
    EXPECT_NE(gendoc_error_category().message(static_cast<int>(File_Read_Failed)), gendoc_error_category().message(0));
    EXPECT_NE(gendoc_error_category().message(static_cast<int>(Create_Output_Dir_Failed)),
              gendoc_error_category().message(0));
    EXPECT_NE(gendoc_error_category().message(static_cast<int>(Protobuf_Parsing_Failed)),
              gendoc_error_category().message(0));
    EXPECT_NE(gendoc_error_category().message(static_cast<int>(Spec_Violated)), gendoc_error_category().message(0));
    EXPECT_NE(gendoc_error_category().message(static_cast<int>(Project_Dir_Does_Not_Exist)),
              gendoc_error_category().message(0));
}

TEST(GenDocCommandTest, Error_Codes_Are_Mapped_To_Appropriate_Error_Conditions)
{
    using enum GenDocErrc;

    EXPECT_EQ(std::error_code(File_Write_Failed), CommandError::File_Operation_Failed);
    EXPECT_EQ(std::error_code(File_Read_Failed), CommandError::File_Operation_Failed);
    EXPECT_EQ(std::error_code(Create_Output_Dir_Failed), CommandError::File_Operation_Failed);
    EXPECT_EQ(std::error_code(Protobuf_Parsing_Failed), CommandError::Protobuf_Parsing_Failed);
    EXPECT_EQ(std::error_code(Spec_Violated), CommandError::Spec_Violated);
    EXPECT_EQ(std::error_code(Project_Dir_Does_Not_Exist), CommandError::Invalid_Argument);
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
