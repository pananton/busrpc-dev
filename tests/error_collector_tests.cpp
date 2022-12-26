#include "commands/check/check_command.h"
#include "commands/imports/imports_command.h"
#include "error_collector.h"
#include "utils.h"

#include <gtest/gtest.h>

#include <sstream>

namespace busrpc { namespace test {

TEST(ErrorCollectorTest, Created_Object_Does_Not_Have_Errors)
{
    ErrorCollector ecol;

    EXPECT_FALSE(ecol);
    EXPECT_FALSE(ecol.majorError());
    EXPECT_TRUE(ecol.errors().empty());
}

TEST(ErrorCollectorTest, Ctor_Wo_Protobuf_Error_Code_Does_Not_Create_Collector_For_Protobuf_Errors)
{
    ErrorCollector ecol;

    EXPECT_EQ(ecol.getProtobufCollector(), nullptr);
}

TEST(ErrorCollectorTest, Ctor_With_Protobuf_Error_Code_Creates_Collector_For_Protobuf_Errors)
{
    ErrorCollector ecol(CheckErrc::Protobuf_Parsing_Failed);

    EXPECT_NE(ecol.getProtobufCollector(), nullptr);
}

TEST(ErrorCollectorTest, add_Stores_Error_And_Updates_Major_Error)
{
    ErrorCollector ecol;

    ASSERT_NO_THROW(ecol.add(CheckErrc::Protobuf_Parsing_Failed));
    ASSERT_TRUE(ecol.majorError());
    EXPECT_TRUE(ecol);
    EXPECT_EQ(ecol.majorError()->code, CheckErrc::Protobuf_Parsing_Failed);
    ASSERT_EQ(ecol.errors().size(), 1);
    EXPECT_EQ(ecol.errors()[0].code, ecol.majorError()->code);
    EXPECT_EQ(ecol.errors()[0].description, ecol.majorError()->description);
}

TEST(ErrorCollectorTest, add_Correctly_Initializes_Error_Description)
{
    std::error_code ec = CheckErrc::Protobuf_Parsing_Failed;
    ErrorCollector ecol;
    ecol.add(ec, "test", std::make_pair("var", 1001));

    ASSERT_TRUE(ecol.majorError());
    EXPECT_NE(ecol.majorError()->description.find(ec.category().name()), std::string::npos);
    EXPECT_NE(ecol.majorError()->description.find(ec.message()), std::string::npos);
    EXPECT_NE(ecol.majorError()->description.find("test"), std::string::npos);
    EXPECT_NE(ecol.majorError()->description.find("var='1001'"), std::string::npos);
    EXPECT_EQ(ecol.errors()[0].description, ecol.majorError()->description);
}

TEST(ErrorCollectorTest, find_Returns_First_Found_Error_Code)
{
    ErrorCollector ecol;
    ecol.add(CheckErrc::File_Read_Failed, "first");
    ecol.add(CheckErrc::File_Read_Failed, "second");

    EXPECT_EQ(ecol.errors().size(), 2);
    ASSERT_TRUE(ecol.find(CheckErrc::File_Read_Failed));
    EXPECT_NE(ecol.find(CheckErrc::File_Read_Failed)->description.find("first"), std::string::npos);
}

TEST(ErrorCollectorTest, find_Returns_Nullopt_If_Error_Code_Was_Not_Added)
{
    ErrorCollector ecol;

    EXPECT_FALSE(ecol.find(ImportsErrc::File_Read_Failed));

    ecol.add(CheckErrc::File_Read_Failed);

    EXPECT_FALSE(ecol.find(ImportsErrc::File_Read_Failed));
}

TEST(ErrorCollectorTest, First_Error_Is_Stored_As_Major_If_No_Ordering_Function_Specified)
{
    ErrorCollector ecol;
    ecol.add(CheckErrc::File_Read_Failed);
    ecol.add(CheckErrc::Spec_Violated, "test");
    ecol.add(CheckErrc::Protobuf_Style_Violated, std::make_pair(1, 1));

    ASSERT_TRUE(ecol.majorError());
    EXPECT_EQ(ecol.majorError()->code, CheckErrc::File_Read_Failed);
}

TEST(ErrorCollectorTest, Most_Severe_Error_Is_Stored_As_Major_As_Determined_By_Specified_Ordering_Function)
{
    ErrorCollector ecol(SeverityByErrorCodeValue);
    ecol.add(CheckErrc::File_Read_Failed);
    ecol.add(CheckErrc::Spec_Violated, "test");
    ecol.add(CheckErrc::Protobuf_Style_Violated, std::make_pair(1, 1));

    ASSERT_TRUE(ecol.majorError());
    EXPECT_EQ(ecol.majorError()->code, CheckErrc::Spec_Violated);
}

TEST(ErrorCollectorTest, add_Ignores_Error_Code_Indicating_Success)
{
    ErrorCollector ecol(SeverityByErrorCodeValue);
    ecol.add(std::error_code(0, check_error_category()));

    EXPECT_EQ(ecol.errors().size(), 0);

    ecol.add(CheckErrc::File_Read_Failed);

    ASSERT_EQ(ecol.errors().size(), 1);
    EXPECT_EQ(ecol.errors()[0].code, CheckErrc::File_Read_Failed);

    ecol.add(std::error_code(0, check_error_category()));

    EXPECT_EQ(ecol.errors().size(), 1);
}

TEST(ErrorCollectorTest, add_Ignores_Error_Code_Whose_Category_Is_Ignored)
{
    ErrorCollector ecol({}, {&imports_error_category()});
    ecol.add(ImportsErrc::File_Not_Found);

    EXPECT_EQ(ecol.errors().size(), 0);

    ecol.add(CheckErrc::File_Read_Failed);

    ASSERT_EQ(ecol.errors().size(), 1);
    EXPECT_EQ(ecol.errors()[0].code, CheckErrc::File_Read_Failed);
}

TEST(ErrorCollectorTest, Error_Codes_Are_Stored_In_Order_Of_Addition)
{
    ErrorCollector ecol;
    ecol.add(CheckErrc::File_Read_Failed);
    ecol.add(CheckErrc::Spec_Violated, "test");
    ecol.add(CheckErrc::Protobuf_Style_Violated, std::make_pair(1, 1));

    ASSERT_EQ(ecol.errors().size(), 3);
    EXPECT_EQ(ecol.errors()[0].code, CheckErrc::File_Read_Failed);
    EXPECT_EQ(ecol.errors()[1].code, CheckErrc::Spec_Violated);
    EXPECT_EQ(ecol.errors()[2].code, CheckErrc::Protobuf_Style_Violated);
}

TEST(ErrorCollectorTest, clear_Removes_All_Error_Codes)
{
    ErrorCollector ecol;
    ecol.add(CheckErrc::File_Read_Failed);
    ecol.add(CheckErrc::Spec_Violated, "test");
    ecol.add(CheckErrc::Protobuf_Style_Violated, std::make_pair(1, 1));

    ASSERT_NO_THROW(ecol.clear());
    EXPECT_FALSE(ecol);
    EXPECT_FALSE(ecol.majorError());
    EXPECT_TRUE(ecol.errors().empty());
}

TEST(ErrorCollectorTest, All_Errors_Are_Outputted_To_Stream)
{
    std::ostringstream out;
    ErrorCollector ecol;
    ecol.add(CheckErrc::File_Read_Failed);
    ecol.add(CheckErrc::Spec_Violated, "test");
    ecol.add(CheckErrc::Protobuf_Style_Violated, std::make_pair(1, 1));

    ASSERT_NO_THROW(out << ecol);

    std::vector<std::string> lines = SplitString(out.str());

    ASSERT_EQ(lines.size(), 3);
    EXPECT_NE(lines[0].find(check_error_category().name()), std::string::npos);
    EXPECT_NE(lines[0].find(check_error_category().message(static_cast<int>(CheckErrc::File_Read_Failed))),
              std::string::npos);
    EXPECT_NE(lines[1].find(check_error_category().message(static_cast<int>(CheckErrc::Spec_Violated))),
              std::string::npos);
    EXPECT_NE(lines[1].find(check_error_category().name()), std::string::npos);
    EXPECT_NE(lines[2].find(check_error_category().message(static_cast<int>(CheckErrc::Protobuf_Style_Violated))),
              std::string::npos);
    EXPECT_NE(lines[2].find(check_error_category().name()), std::string::npos);
}

TEST(ErrorCollectorTest, Collector_Guard_Outputs_Errors_In_Destructor)
{
    std::ostringstream out;
    ErrorCollector ecol;

    {
        ErrorCollectorGuard guard(ecol, out);
        ecol.add(CheckErrc::File_Read_Failed);
        ecol.add(CheckErrc::Spec_Violated, "test");
        ecol.add(CheckErrc::Protobuf_Style_Violated, std::make_pair(1, 1));
    }

    std::vector<std::string> lines = SplitString(out.str());

    EXPECT_EQ(ecol.errors().size(), 3);
    ASSERT_EQ(lines.size(), 3);
    EXPECT_NE(lines[0].find(check_error_category().message(static_cast<int>(CheckErrc::File_Read_Failed))),
              std::string::npos);
    EXPECT_NE(lines[1].find(check_error_category().message(static_cast<int>(CheckErrc::Spec_Violated))),
              std::string::npos);
    EXPECT_NE(lines[2].find(check_error_category().message(static_cast<int>(CheckErrc::Protobuf_Style_Violated))),
              std::string::npos);
}
}} // namespace busrpc::test
