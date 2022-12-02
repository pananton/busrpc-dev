#include "app.h"
#include "commands/check/check_command.h"
#include "commands/help/help_command.h"
#include "utils/common.h"

#include <CLI/CLI.hpp>
#include <gtest/gtest.h>

#include <sstream>

namespace busrpc { namespace test {

TEST(CheckCommandTest, Command_Name_And_Id_Are_Mapped_To_Each_Other)
{
    EXPECT_EQ(CommandId::Check, GetCommandId(GetCommandName(CommandId::Check)));
    EXPECT_EQ(CheckCommand::Id, CommandId::Check);
    EXPECT_STREQ(CheckCommand::Name, GetCommandName(CommandId::Check));
}

TEST(CheckCommandTest, Command_Error_Category_Name_Matches_Command_Name)
{
    EXPECT_STREQ(check_error_category().name(), GetCommandName(CommandId::Check));
}

TEST(CheckCommandTest, Description_For_Unknown_Command_Error_Code_Is_Not_Empty)
{
    EXPECT_FALSE(check_error_category().message(0).empty());
}

TEST(CheckCommandTest, Description_For_Unknown_Command_Error_Code_Differs_From_Known_Error_Codes_Descriptions)
{
    EXPECT_NE(check_error_category().message(static_cast<int>(CheckErrc::File_Read_Failed)),
              check_error_category().message(0));
    EXPECT_NE(check_error_category().message(static_cast<int>(CheckErrc::Protobuf_Parsing_Failed)),
              check_error_category().message(0));
    EXPECT_NE(check_error_category().message(static_cast<int>(CheckErrc::Protobuf_Style_Violated)),
              check_error_category().message(0));
    EXPECT_NE(check_error_category().message(static_cast<int>(CheckErrc::Undocumeted_Entity)),
              check_error_category().message(0));
    EXPECT_NE(check_error_category().message(static_cast<int>(CheckErrc::Spec_Violated)),
              check_error_category().message(0));
    EXPECT_NE(check_error_category().message(static_cast<int>(CheckErrc::Project_Dir_Does_Not_Exist)),
              check_error_category().message(0));
}

TEST(CheckCommandTest, Error_Codes_Are_Mapped_To_Appropriate_Error_Conditions)
{
    EXPECT_EQ(std::error_code(CheckErrc::File_Read_Failed), CommandError::File_Operation_Failed);
    EXPECT_EQ(std::error_code(CheckErrc::Protobuf_Parsing_Failed), CommandError::Protobuf_Parsing_Failed);
    EXPECT_EQ(std::error_code(CheckErrc::Protobuf_Style_Violated), CommandError::Spec_Violated);
    EXPECT_EQ(std::error_code(CheckErrc::Undocumeted_Entity), CommandError::Spec_Violated);
    EXPECT_EQ(std::error_code(CheckErrc::Spec_Violated), CommandError::Spec_Violated);
    EXPECT_EQ(std::error_code(CheckErrc::Project_Dir_Does_Not_Exist), CommandError::Invalid_Argument);
}

TEST(CheckCommandTest, Help_Is_Defined_For_The_Command)
{
    HelpCommand helpCmd({CommandId::Check});
    std::ostringstream out, err;

    EXPECT_NO_THROW(helpCmd.execute(out, err));
    EXPECT_TRUE(IsHelpMessage(out.str(), CommandId::Check));
    EXPECT_TRUE(err.str().empty());
}
}} // namespace busrpc::test
