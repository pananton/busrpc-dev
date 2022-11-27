#include "commands/check/check_command.h"
#include "commands/imports/imports_command.h"
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
    ErrorCollector col(check_error_category(), CheckErrc::Protobuf_Parsing_Failed);

    EXPECT_NE(col.getProtobufCollector(), nullptr);
}

TEST(ErrorCollectorTest, Final_Error_Code_Indicates_Success_If_No_Errors_Were_Added)
{
    ErrorCollector col(check_error_category());

    EXPECT_FALSE(col.result());
}

TEST(ErrorCollectorTest, Default_Description_Is_Outputted_To_Error_Stream_If_No_Custom_Message_Specified)
{
    std::ostringstream err;
    ErrorCollector col(check_error_category(), err);

    EXPECT_NO_THROW(col.add(CheckErrc::File_Read_Failed));
    EXPECT_EQ(err.str().find(check_error_category().message(static_cast<int>(CheckErrc::File_Read_Failed))), 0);
}

TEST(ErrorCollectorTest, Custom_Message_Is_Outputted_To_Error_Stream_If_Specified)
{
    const char* msg = "my message";
    std::ostringstream err;
    ErrorCollector col(check_error_category(), err);

    EXPECT_NO_THROW(col.add(CheckErrc::File_Read_Failed, msg));
    EXPECT_EQ(err.str().find(check_error_category().message(static_cast<int>(CheckErrc::File_Read_Failed))),
              std::string::npos);
    EXPECT_NE(err.str().find(msg), std::string::npos);
}

TEST(ErrorCollectorTest, Error_Code_With_Biggest_Value_Is_Used_As_Result)
{
    std::ostringstream err;
    ErrorCollector col(check_error_category(), err);

    EXPECT_NO_THROW(col.add(CheckErrc::File_Read_Failed));
    EXPECT_EQ(col.result(), CheckErrc::File_Read_Failed);
    EXPECT_NO_THROW(col.add(CheckErrc::Undocumeted_Entity));
    EXPECT_EQ(col.result(), CheckErrc::Undocumeted_Entity);
    EXPECT_NO_THROW(col.add(CheckErrc::Protobuf_Style_Violated));
    EXPECT_EQ(col.result(), CheckErrc::Undocumeted_Entity);
    EXPECT_NO_THROW(col.add(CheckErrc::Root_Does_Not_Exist));
    EXPECT_EQ(col.result(), CheckErrc::Root_Does_Not_Exist);
}

TEST(ErrorCollectorTest, Success_Code_Is_Ignored)
{
    std::ostringstream err;
    ErrorCollector col(check_error_category(), err);

    EXPECT_NO_THROW(col.add(CheckErrc::Undocumeted_Entity));
    EXPECT_EQ(col.result(), CheckErrc::Undocumeted_Entity);

    err.str("");

    EXPECT_NO_THROW(col.add({0, check_error_category()}));
    EXPECT_EQ(col.result(), CheckErrc::Undocumeted_Entity);
    EXPECT_TRUE(err.str().empty());
}

TEST(ErrorCollectorTest, Error_Code_With_Different_Category_Is_Ignored)
{
    std::ostringstream err;
    ErrorCollector col(check_error_category(), err);

    EXPECT_NO_THROW(col.add(CheckErrc::Undocumeted_Entity));
    EXPECT_EQ(col.result(), CheckErrc::Undocumeted_Entity);

    err.str("");

    EXPECT_NO_THROW(col.add(ImportsErrc::Root_Does_Not_Exist));
    EXPECT_EQ(col.result(), CheckErrc::Undocumeted_Entity);
    EXPECT_TRUE(err.str().empty());
}

TEST(ErrorCollectorTest, Description_For_Error_Code_With_Smaller_Value_Than_Current_Result_Is_Outputted)
{
    const char* msg = "my message";
    std::ostringstream err;
    ErrorCollector col(check_error_category(), err);

    EXPECT_NO_THROW(col.add(CheckErrc::Undocumeted_Entity));
    EXPECT_EQ(col.result(), CheckErrc::Undocumeted_Entity);

    err.str("");

    EXPECT_NO_THROW(col.add(CheckErrc::Protobuf_Style_Violated));
    EXPECT_EQ(col.result(), CheckErrc::Undocumeted_Entity);
    EXPECT_EQ(err.str().find(check_error_category().message(static_cast<int>(CheckErrc::Protobuf_Style_Violated))), 0);

    err.str("");

    EXPECT_NO_THROW(col.add(CheckErrc::File_Read_Failed, msg));
    EXPECT_EQ(col.result(), CheckErrc::Undocumeted_Entity);
    EXPECT_NE(err.str().find(msg), std::string::npos);
}
}} // namespace busrpc::test