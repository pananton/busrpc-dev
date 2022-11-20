#include "commands/command.h"

#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>
#include <string>

namespace busrpc { namespace test {

constexpr CommandId TestCommandId = CommandId::Version;

struct TestCommandArgs {
    std::string output = "";
    std::string error = "";
    bool throws = false;
};

bool operator==(const TestCommandArgs& lhs, const TestCommandArgs& rhs)
{
    return lhs.output == rhs.output && lhs.error == rhs.error && lhs.throws == rhs.throws;
}

bool operator!=(const TestCommandArgs& lhs, const TestCommandArgs& rhs)
{
    return !(lhs == rhs);
}

class TestCommand: public Command<TestCommandId, TestCommandArgs> {
public:
    using Base = Command<TestCommandId, TestCommandArgs>;

    TestCommand(TestCommandArgs args): Base(std::move(args)) { }

protected:
    std::error_code executeImpl(std::ostream& out, std::ostream& err) const override
    {
        if (!args().output.empty()) {
            out << args().output;
        }

        if (!args().error.empty()) {
            err << args().error;
        }

        if (args().throws) {
            throw std::runtime_error("expected");
        }

        return {0, std::generic_category()};
    }
};

TEST(CommandTest, Get_Command_Name_Returns_Nullptr_For_Unknown_Command)
{
    EXPECT_EQ(GetCommandName(static_cast<CommandId>(0)), nullptr);
}

TEST(CommandTest, Get_Command_Id_Returns_Zero_For_Unknown_Command_Name)
{
    EXPECT_EQ(GetCommandId("unknown command"), static_cast<CommandId>(0));
}

TEST(CommandTest, Command_Error_Category_Is_command)
{
    EXPECT_EQ(command_error_category().name(), "command");
}

TEST(CommandTest, Description_For_Unknown_Command_Error_Code_Exists_And_Differs_From_Known_Error_Codes_Descriptions)
{
    EXPECT_FALSE(command_error_category().message(0).empty());
    EXPECT_NE(command_error_category().message(static_cast<int>(CommandError::Internal)),
              command_error_category().message(0));
    EXPECT_NE(command_error_category().message(static_cast<int>(CommandError::Logic)),
              command_error_category().message(0));
    EXPECT_NE(command_error_category().message(static_cast<int>(CommandError::Invalid_Argument)),
              command_error_category().message(0));
    EXPECT_NE(command_error_category().message(static_cast<int>(CommandError::File_Operation_Failed)),
              command_error_category().message(0));
    EXPECT_NE(command_error_category().message(static_cast<int>(CommandError::Protobuf_Syntax_Error)),
              command_error_category().message(0));
}

TEST(CommandTest, Command_Ctor_Sets_Command_Id_And_Name)
{
    auto cmd = TestCommand({});

    EXPECT_EQ(cmd.id(), TestCommandId);
    EXPECT_STREQ(cmd.name(), GetCommandName(TestCommandId));
}

TEST(CommandTest, Command_Ctor_Sets_Args)
{
    TestCommandArgs args{"hello", "error occurred", false};

    EXPECT_EQ(TestCommand(args).args(), args);
}

TEST(CommandTest, Output_And_Error_Streams_Are_Passed_To_Execute_Implementation)
{
    std::string output = "hello";
    std::string error = "error";
    TestCommand cmd({output, error, false});
    std::ostringstream out, err;

    ASSERT_NO_THROW(cmd.execute(out, err));
    EXPECT_EQ(out.str(), output);
    EXPECT_EQ(err.str(), error);
}

TEST(CommandTest, Same_Stream_Can_Be_Used_For_Output_And_Error_In_Execute)
{
    std::string output = "hello";
    std::string error = "error";
    TestCommand cmd({output, error, false});
    std::ostringstream out;

    ASSERT_NO_THROW(cmd.execute(out, out));
    EXPECT_EQ(out.str(), output + error);
}

TEST(CommandTest, Execute_Implementation_Is_Invoked_When_Output_Stream_Is_Not_Set)
{
    std::string output = "hello";
    std::string error = "error";
    TestCommand cmd({output, error, false});
    std::ostringstream err;

    ASSERT_NO_THROW(cmd.execute(std::nullopt, err));
    EXPECT_EQ(err.str(), error);
}

TEST(CommandTest, Execute_Implementation_Is_Invoked_When_Error_Stream_Is_Not_Set)
{
    std::string output = "hello";
    std::string error = "error";
    TestCommand cmd({output, error, false});
    std::ostringstream out;

    ASSERT_NO_THROW(cmd.execute(out, std::nullopt));
    EXPECT_EQ(out.str(), output);
}

TEST(CommandTest, Execute_Implementation_Is_Invoked_When_Both_Streams_Are_Not_Set)
{
    std::string output = "hello";
    std::string error = "error";
    TestCommand cmd({output, error, true});

    ASSERT_THROW(cmd.execute(std::nullopt, std::nullopt), std::runtime_error);
}
}} // namespace busrpc::test
