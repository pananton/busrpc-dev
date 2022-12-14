#include "commands/imports/imports_command.h"
#include "exception.h"

#include <gtest/gtest.h>

#include <string_view>

namespace busrpc { namespace test {

TEST(ExceptionTest, Command_Error_Ctor_Correctly_Initializes_Object)
{
    command_error err(CommandId::Imports, ImportsErrc::File_Not_Found);

    EXPECT_EQ(err.commandId(), CommandId::Imports);
    EXPECT_EQ(err.code(), ImportsErrc::File_Not_Found);
}

TEST(ExceptionTest, Command_Error_Ctor_Adds_Command_Name_To_Error_Description)
{
    command_error err(CommandId::Imports, ImportsErrc::File_Not_Found);
    std::string_view description(err.what());

    EXPECT_NE(description.find(GetCommandName(CommandId::Imports)), std::string_view::npos);
}

TEST(ExceptionTest, Entity_Error_Ctor_Correctly_Initializes_Object)
{
    std::string_view dname = "aaa.bbb.ccc";
    entity_error err(EntityTypeId::Class, dname, "some reason");

    EXPECT_EQ(err.type(), EntityTypeId::Class);
    EXPECT_EQ(err.dname(), dname);
}

TEST(ExceptionTest, Entity_Error_Constructor_Adds_Entity_Type_Dname_And_Error_Reason_To_Error_Description)
{
    std::string_view dname = "aaa.bbb.ccc";
    std::string_view reason = "some reason";
    entity_error err(EntityTypeId::Class, dname, reason);
    std::string_view description(err.what());

    EXPECT_NE(description.find(GetEntityTypeIdStr(EntityTypeId::Class)), std::string_view::npos);
    EXPECT_NE(description.find(dname), std::string_view::npos);
    EXPECT_NE(description.find(reason), std::string_view::npos);
}

TEST(ExceptionTest, Name_Conflict_Error_Ctor_Correctly_Initializes_Object)
{
    std::string_view dname = "aaa.bbb.ccc";
    std::string_view conflictingName = "conflicting_name";
    name_conflict_error err(EntityTypeId::Class, dname, conflictingName);

    EXPECT_EQ(err.type(), EntityTypeId::Class);
    EXPECT_EQ(err.dname(), dname);
    EXPECT_EQ(err.conflictingName(), conflictingName);
}

TEST(ExceptionTest, Name_Conflict_Error_Constructor_Adds_Conflicting_Name_To_Error_Description)
{
    std::string_view conflictingName = "conflicting_name";
    name_conflict_error err(EntityTypeId::Class, "aaa.bbb.ccc", conflictingName);
    std::string_view description(err.what());

    EXPECT_NE(description.find(conflictingName), std::string_view::npos);
}
}} // namespace busrpc::test
