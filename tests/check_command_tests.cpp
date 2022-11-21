#include "commands/check/check_command.h"

#include <gtest/gtest.h>

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
    EXPECT_NE(check_error_category().message(static_cast<int>(CheckErrc::Non_Existent_Root_Error)),
              check_error_category().message(0));
    EXPECT_NE(check_error_category().message(static_cast<int>(CheckErrc::Specification_Error)),
              check_error_category().message(0));
    EXPECT_NE(check_error_category().message(static_cast<int>(CheckErrc::Docs_Error)),
              check_error_category().message(0));
    EXPECT_NE(check_error_category().message(static_cast<int>(CheckErrc::Style_Error)),
              check_error_category().message(0));
    EXPECT_NE(check_error_category().message(static_cast<int>(CheckErrc::Protobuf_Error)),
              check_error_category().message(0));
    EXPECT_NE(check_error_category().message(static_cast<int>(CheckErrc::File_Read_Error)),
              check_error_category().message(0));
}

TEST(CheckCommandTest, Error_Codes_Are_Mapped_To_Appropriate_Error_Conditions)
{
    EXPECT_EQ(std::error_code(CheckErrc::Non_Existent_Root_Error), CommandError::Argument_Error);
    EXPECT_EQ(std::error_code(CheckErrc::Specification_Error), CommandError::Logic_Error);
    EXPECT_EQ(std::error_code(CheckErrc::Docs_Error), CommandError::Logic_Error);
    EXPECT_EQ(std::error_code(CheckErrc::Style_Error), CommandError::Logic_Error);
    EXPECT_EQ(std::error_code(CheckErrc::Protobuf_Error), CommandError::Protobuf_Error);
    EXPECT_EQ(std::error_code(CheckErrc::File_Read_Error), CommandError::File_Access_Error);
}
}} // namespace busrpc::test
