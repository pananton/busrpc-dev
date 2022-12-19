#include "commands/check/check_command.h"
#include "error_collector.h"
#include "utils.h"

#include <gtest/gtest.h>

#include <sstream>

namespace busrpc { namespace test {

TEST(ErrorCollectorTest, Created_Object_Does_Not_Have_Errors)
{
    ErrorCollector col;

    EXPECT_FALSE(col);
    EXPECT_FALSE(col.majorError());
    EXPECT_TRUE(col.errors().empty());
}

TEST(ErrorCollectorTest, Ctor_Wo_Protobuf_Error_Code_Does_Not_Create_Collector_For_Protobuf_Errors)
{
    ErrorCollector col;

    EXPECT_EQ(col.getProtobufCollector(), nullptr);
}

TEST(ErrorCollectorTest, Ctor_With_Protobuf_Error_Code_Creates_Collector_For_Protobuf_Errors)
{
    ErrorCollector col(CheckErrc::Protobuf_Parsing_Failed);

    EXPECT_NE(col.getProtobufCollector(), nullptr);
}

TEST(ErrorCollectorTest, add_Stores_Error_And_Updates_Major_Error)
{
    ErrorCollector col;

    ASSERT_NO_THROW(col.add(CheckErrc::Protobuf_Parsing_Failed));
    ASSERT_TRUE(col.majorError());
    EXPECT_TRUE(col);
    EXPECT_EQ(col.majorError()->code, CheckErrc::Protobuf_Parsing_Failed);
    ASSERT_EQ(col.errors().size(), 1);
    EXPECT_EQ(col.errors()[0].code, col.majorError()->code);
    EXPECT_EQ(col.errors()[0].description, col.majorError()->description);
}

TEST(ErrorCollectorTest, add_Correctly_Initializes_Error_Description)
{
    std::error_code ec = CheckErrc::Protobuf_Parsing_Failed;
    ErrorCollector col;
    col.add(ec, "test", std::make_pair("var", 1001));

    ASSERT_TRUE(col.majorError());
    EXPECT_NE(col.majorError()->description.find(ec.message()), std::string::npos);
    EXPECT_NE(col.majorError()->description.find("test"), std::string::npos);
    EXPECT_NE(col.majorError()->description.find("var='1001'"), std::string::npos);
    EXPECT_EQ(col.errors()[0].description, col.majorError()->description);
}

TEST(ErrorCollectorTest, First_Error_Is_Stored_As_Major_If_No_Ordering_Function_Specified)
{
    ErrorCollector col;
    col.add(CheckErrc::File_Read_Failed);
    col.add(CheckErrc::Spec_Violated, "test");
    col.add(CheckErrc::Protobuf_Style_Violated, std::make_pair(1, 1));

    ASSERT_TRUE(col.majorError());
    EXPECT_EQ(col.majorError()->code, CheckErrc::File_Read_Failed);
}

TEST(ErrorCollectorTest, Most_Severe_Error_Is_Stored_As_Major_As_Determined_By_Specified_Ordering_Function)
{
    ErrorCollector col(SeverityByErrorCodeValue);
    col.add(CheckErrc::File_Read_Failed);
    col.add(CheckErrc::Spec_Violated, "test");
    col.add(CheckErrc::Protobuf_Style_Violated, std::make_pair(1, 1));

    ASSERT_TRUE(col.majorError());
    EXPECT_EQ(col.majorError()->code, CheckErrc::Spec_Violated);
}

TEST(ErrorCollectorTest, add_Ignores_Error_Code_Indicating_Success)
{
    ErrorCollector col(SeverityByErrorCodeValue);
    col.add(std::error_code(0, check_error_category()));

    EXPECT_FALSE(col);

    col.add(CheckErrc::File_Read_Failed);

    ASSERT_TRUE(col.majorError());
    EXPECT_EQ(col.majorError()->code, CheckErrc::File_Read_Failed);

    col.add(std::error_code(0, check_error_category()));

    ASSERT_TRUE(col.majorError());
    EXPECT_EQ(col.majorError()->code, CheckErrc::File_Read_Failed);
}
TEST(ErrorCollectorTest, Error_Codes_Are_Stored_In_Order_Of_Addition)
{
    ErrorCollector col;
    col.add(CheckErrc::File_Read_Failed);
    col.add(CheckErrc::Spec_Violated, "test");
    col.add(CheckErrc::Protobuf_Style_Violated, std::make_pair(1, 1));

    ASSERT_EQ(col.errors().size(), 3);
    EXPECT_EQ(col.errors()[0].code, CheckErrc::File_Read_Failed);
    EXPECT_EQ(col.errors()[1].code, CheckErrc::Spec_Violated);
    EXPECT_EQ(col.errors()[2].code, CheckErrc::Protobuf_Style_Violated);
}

TEST(ErrorCollectorTest, clear_Removes_All_Error_Codes)
{
    ErrorCollector col;
    col.add(CheckErrc::File_Read_Failed);
    col.add(CheckErrc::Spec_Violated, "test");
    col.add(CheckErrc::Protobuf_Style_Violated, std::make_pair(1, 1));

    ASSERT_NO_THROW(col.clear());
    EXPECT_FALSE(col);
    EXPECT_FALSE(col.majorError());
    EXPECT_TRUE(col.errors().empty());
}

TEST(ErrorCollectorTest, All_Errors_Are_Outputted_To_Stream)
{
    std::ostringstream out;
    ErrorCollector col;
    col.add(CheckErrc::File_Read_Failed);
    col.add(CheckErrc::Spec_Violated, "test");
    col.add(CheckErrc::Protobuf_Style_Violated, std::make_pair(1, 1));

    ASSERT_NO_THROW(out << col);

    std::vector<std::string> lines = SplitString(out.str());

    ASSERT_EQ(lines.size(), 3);
    EXPECT_NE(lines[0].find(check_error_category().message(static_cast<int>(CheckErrc::File_Read_Failed))),
              std::string::npos);
    EXPECT_NE(lines[1].find(check_error_category().message(static_cast<int>(CheckErrc::Spec_Violated))),
              std::string::npos);
    EXPECT_NE(lines[2].find(check_error_category().message(static_cast<int>(CheckErrc::Protobuf_Style_Violated))),
              std::string::npos);
}
}} // namespace busrpc::test
