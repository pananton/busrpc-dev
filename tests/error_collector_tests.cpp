#include "commands/check/check_command.h"
#include "commands/configure/configure_command.h"
#include "commands/error_collector.h"

#include <gtest/gtest.h>

#include <sstream>

namespace busrpc { namespace test {

TEST(ErrorCollectorTest, Ctor_Wo_Protobuf_Error_Code_Does_Not_Create_Collector_For_Protobuf_Errors)
{
    std::ostringstream err;
    ErrorCollector col(check_error_category(), err);

    EXPECT_EQ(col.getProtobufCollector(), nullptr);
}

TEST(ErrorCollectorTest, Ctor_With_Protobuf_Error_Code_Creates_Collector_For_Protobuf_Errors)
{
    ErrorCollector col(check_error_category(), CheckErrc::Protobuf_Error);

    EXPECT_NE(col.getProtobufCollector(), nullptr);
}

TEST(ErrorCollectorTest, Final_Error_Code_Indicates_Success_If_No_Errors_Were_Added)
{
    ErrorCollector col(check_error_category());

    EXPECT_FALSE(col.finalError());
}

TEST(ErrorCollectorTest, Default_Description_Is_Outputted_To_Error_Stream_When_Error_Is_Added)
{
    std::ostringstream err;
    ErrorCollector col(check_error_category(), err);

    EXPECT_NO_THROW(col.add(CheckErrc::File_Read_Error));
    EXPECT_EQ(err.str().find(check_error_category().message(static_cast<int>(CheckErrc::File_Read_Error))), 0);
}

TEST(ErrorCollectorTest, Custom_Message_And_Default_Description_Is_Outputted_To_Error_Stream_When_Error_Is_Added)
{
    const char* msg = "my message";
    std::ostringstream err;
    ErrorCollector col(check_error_category(), err);

    EXPECT_NO_THROW(col.add(CheckErrc::File_Read_Error, msg));
    EXPECT_EQ(err.str().find(check_error_category().message(static_cast<int>(CheckErrc::File_Read_Error))), 0);
    EXPECT_NE(err.str().find(msg), std::string::npos);
}

TEST(ErrorCollectorTest, Error_Code_With_Smallest_Value_Is_Used_As_Final)
{
    std::ostringstream err;
    ErrorCollector col(check_error_category(), err);

    EXPECT_NO_THROW(col.add(CheckErrc::File_Read_Error));
    EXPECT_EQ(col.finalError(), CheckErrc::File_Read_Error);
    EXPECT_NO_THROW(col.add(CheckErrc::Docs_Error));
    EXPECT_EQ(col.finalError(), CheckErrc::Docs_Error);
    EXPECT_NO_THROW(col.add(CheckErrc::Style_Error));
    EXPECT_EQ(col.finalError(), CheckErrc::Docs_Error);
    EXPECT_NO_THROW(col.add(CheckErrc::Non_Existent_Root_Error));
    EXPECT_EQ(col.finalError(), CheckErrc::Non_Existent_Root_Error);
}

TEST(ErrorCollectorTest, Success_Code_Is_Ignored)
{
    std::ostringstream err;
    ErrorCollector col(check_error_category(), err);

    EXPECT_NO_THROW(col.add(CheckErrc::Docs_Error));
    EXPECT_EQ(col.finalError(), CheckErrc::Docs_Error);

    err.str("");

    EXPECT_NO_THROW(col.add({0, check_error_category()}));
    EXPECT_EQ(col.finalError(), CheckErrc::Docs_Error);
    EXPECT_TRUE(err.str().empty());
}

TEST(ErrorCollectorTest, Error_Code_With_Distinct_Category_Is_Ignored)
{
    std::ostringstream err;
    ErrorCollector col(check_error_category(), err);

    EXPECT_NO_THROW(col.add(CheckErrc::Docs_Error));
    EXPECT_EQ(col.finalError(), CheckErrc::Docs_Error);

    err.str("");

    EXPECT_NO_THROW(col.add(ConfigureErrc::Non_Existent_Root_Error));
    EXPECT_EQ(col.finalError(), CheckErrc::Docs_Error);
    EXPECT_TRUE(err.str().empty());
}

TEST(ErrorCollectorTest, Description_For_Error_Code_With_Higher_Value_Than_Current_Final_Is_Outputted)
{
    const char* msg = "my message";
    std::ostringstream err;
    ErrorCollector col(check_error_category(), err);

    EXPECT_NO_THROW(col.add(CheckErrc::Docs_Error));
    EXPECT_EQ(col.finalError(), CheckErrc::Docs_Error);

    err.str("");

    EXPECT_NO_THROW(col.add(CheckErrc::Protobuf_Error, msg));
    EXPECT_EQ(col.finalError(), CheckErrc::Docs_Error);
    EXPECT_EQ(err.str().find(check_error_category().message(static_cast<int>(CheckErrc::Protobuf_Error))), 0);
    EXPECT_NE(err.str().find(msg), std::string::npos);
}
}} // namespace busrpc::test