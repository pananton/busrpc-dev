#include "commands/imports/imports_command.h"
#include "utils/common.h"

#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>
#include <string>

namespace busrpc { namespace test {

constexpr CommandId TestCommandId = CommandId::Imports;

struct TestCommandArgs {
    std::string output = "";
    std::string error = "";
    std::error_code result = {0, imports_error_category()};
};

bool operator==(const TestCommandArgs& lhs, const TestCommandArgs& rhs)
{
    return lhs.output == rhs.output && lhs.error == rhs.error && lhs.result == rhs.result;
}

bool operator!=(const TestCommandArgs& lhs, const TestCommandArgs& rhs)
{
    return !(lhs == rhs);
}

class TestCommand: public Command<TestCommandId, TestCommandArgs> {
public:
    using Base = Command<TestCommandId, TestCommandArgs>;

    TestCommand(TestCommandArgs args): Base(std::move(args)) { }
    bool isExecuted() const { return isExecuted_; }

protected:
    std::error_code tryExecuteImpl(std::ostream& out, std::ostream& err) const override
    {
        isExecuted_ = true;

        if (!args().output.empty()) {
            out << args().output;
        }

        if (!args().error.empty()) {
            err << args().error;
        }

        return args().result;
    }

private:
    mutable bool isExecuted_ = false;
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
    EXPECT_STREQ(command_error_category().name(), "command");
}

TEST(CommandTest, Command_Error_Ctor_Sets_Command_Id)
{
    EXPECT_EQ(command_error(TestCommand::Id, ImportsErrc::Root_Does_Not_Exist).commandId(), TestCommand::Id);
}

TEST(CommandTest, Command_Error_Ctor_Sets_Error_Code)
{
    EXPECT_EQ(command_error(TestCommand::Id, ImportsErrc::Root_Does_Not_Exist).code(),
              ImportsErrc::Root_Does_Not_Exist);
}

TEST(CommandTest, Command_Error_Ctor_Adds_Command_Name_To_Error_Description)
{
    command_error err(TestCommand::Id, ImportsErrc::Root_Does_Not_Exist);

    EXPECT_NE(std::string_view(err.what()).find(GetCommandName(TestCommand::Id)), std::string_view::npos);
}

TEST(CommandTest, Description_For_Unknown_Command_Error_Condition_Is_Not_Empty)
{
    EXPECT_FALSE(command_error_category().message(0).empty());
}

TEST(CommandTest, Description_For_Unknown_Command_Error_Condition_Differs_From_Known_Error_Conditions_Descriptions)
{
    EXPECT_NE(command_error_category().message(static_cast<int>(CommandError::Internal)),
              command_error_category().message(0));
    EXPECT_NE(command_error_category().message(static_cast<int>(CommandError::File_Operation_Failed)),
              command_error_category().message(0));
    EXPECT_NE(command_error_category().message(static_cast<int>(CommandError::Protobuf_Parsing_Failed)),
              command_error_category().message(0));
    EXPECT_NE(command_error_category().message(static_cast<int>(CommandError::Spec_Violated)),
              command_error_category().message(0));
    EXPECT_NE(command_error_category().message(static_cast<int>(CommandError::Invalid_Argument)),
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
    TestCommandArgs args{"hello", "error occurred"};

    EXPECT_EQ(TestCommand(args).args(), args);
}

TEST(CommandTest, Specified_Output_And_Error_Streams_Are_Passed_To_Implementation)
{
    std::string output = "hello";
    std::string error = "error";
    TestCommand cmd({output, error});
    std::ostringstream out, err;

    EXPECT_NO_THROW(cmd.execute(out, err));
    EXPECT_EQ(out.str(), output);
    EXPECT_EQ(err.str(), error);

    out.str("");
    err.str("");

    EXPECT_FALSE(cmd.tryExecute(out, err));
    EXPECT_EQ(out.str(), output);
    EXPECT_EQ(err.str(), error);
}

TEST(CommandTest, Same_Stream_Can_Be_Used_For_Output_And_Error)
{
    std::string output = "hello";
    std::string error = "error";
    TestCommand cmd({output, error});
    std::ostringstream out;

    EXPECT_NO_THROW(cmd.execute(out, out));
    EXPECT_EQ(out.str(), output + error);

    out.str("");

    EXPECT_FALSE(cmd.tryExecute(out, out));
    EXPECT_EQ(out.str(), output + error);
}

TEST(CommandTest, Command_Is_Executed_Even_If_Output_Stream_Is_Not_Set)
{
    std::string output = "hello";
    std::string error = "error";
    TestCommand cmd({output, error});
    std::ostringstream err;

    EXPECT_NO_THROW(cmd.execute(std::nullopt, err));
    EXPECT_EQ(err.str(), error);
    EXPECT_TRUE(cmd.isExecuted());

    cmd = TestCommand({output, error});
    err.str("");

    EXPECT_FALSE(cmd.tryExecute(std::nullopt, err));
    EXPECT_EQ(err.str(), error);
    EXPECT_TRUE(cmd.isExecuted());
}

TEST(CommandTest, Command_Is_Executed_Even_If_Error_Stream_Is_Not_Set)
{
    std::string output = "hello";
    std::string error = "error";
    TestCommand cmd({output, error});
    std::ostringstream out;

    EXPECT_NO_THROW(cmd.execute(out, std::nullopt));
    EXPECT_EQ(out.str(), output);
    EXPECT_TRUE(cmd.isExecuted());

    cmd = TestCommand({output, error});
    out.str("");

    EXPECT_FALSE(cmd.tryExecute(out, std::nullopt));
    EXPECT_EQ(out.str(), output);
    EXPECT_TRUE(cmd.isExecuted());
}

TEST(CommandTest, Command_Is_Executed_Even_If_Output_And_Error_Streams_Are_Not_Set)
{
    std::string output = "hello";
    std::string error = "error";
    TestCommand cmd({output, error});

    EXPECT_NO_THROW(cmd.execute(std::nullopt, std::nullopt));
    EXPECT_TRUE(cmd.isExecuted());

    cmd = TestCommand({output, error});

    EXPECT_FALSE(cmd.tryExecute(std::nullopt, std::nullopt));
    EXPECT_TRUE(cmd.isExecuted());
}

TEST(CommandTest, Execute_Throws_Command_Error_If_Command_Fails)
{
    TestCommand cmd({"", "", ImportsErrc::File_Not_Found});

    EXPECT_COMMAND_EXCEPTION(cmd.execute(std::nullopt, std::nullopt), ImportsErrc::File_Not_Found);
}
}} // namespace busrpc::test
