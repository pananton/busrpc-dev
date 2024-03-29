#include "entities/project.h"
#include "utils/common.h"

#include <gtest/gtest.h>

#include <memory>

namespace busrpc { namespace test {

constexpr const char* Test_Root = "test_root";

class ProjectEntityTest: public ::testing::Test {
protected:
    void SetUp() override
    {
        project_ = std::make_shared<Project>(Test_Root);
        api_ = project_->addApi();
        implementation_ = project_->addImplementation();

        ns1_ = api_->addNamespace("ns1");
        ns2_ = api_->addNamespace("ns2");

        cls1_ = ns1_->addClass("cls1");
        method1_ = cls1_->addMethod("method1");
        method2_ = cls1_->addMethod("method2");
        enum1_ = method1_->addEnum("Enum1", "file1.proto");
        constant1_ = enum1_->addConstant("constant1", 1);

        struct1_ = ns2_->addStruct("Struct1", "file2.proto");
        field1_ = struct1_->addScalarField("field1", 1, FieldTypeId::Bool);
        nestedStruct1_ = struct1_->addStruct("NestedStruct1");
        nestedStruct1_->addScalarField("field1", 1, FieldTypeId::Bool);
        nestedEnum1_ = struct1_->addEnum("NestedEnum1");

        implementationStruct1_ = implementation_->addStruct("Struct1", "file3.proto");
        service_ = implementation_->addService("service");
        serviceEnum1_ = service_->addEnum("Enum1", "file4.proto");
    }

protected:
    std::shared_ptr<Project> project_;
    Api* api_ = nullptr;
    Implementation* implementation_ = nullptr;

    // api types

    Namespace* ns1_ = nullptr;
    Namespace* ns2_ = nullptr;
    Class* cls1_ = nullptr;
    Method* method1_ = nullptr;
    Method* method2_ = nullptr;
    Struct* struct1_ = nullptr;
    Field* field1_ = nullptr;
    Struct* nestedStruct1_ = nullptr;
    Enum* enum1_ = nullptr;
    Constant* constant1_ = nullptr;
    Enum* nestedEnum1_ = nullptr;

    // services types

    Service* service_ = nullptr;
    Struct* implementationStruct1_ = nullptr;
    Enum* serviceEnum1_ = nullptr;
};

TEST_F(ProjectEntityTest, Ctor_Correctly_Initializes_Project_Entity)
{
    EXPECT_EQ(project_->type(), EntityTypeId::Project);
    EXPECT_EQ(project_->name(), Project_Entity_Name);
    EXPECT_EQ(project_->dir(), "");
    EXPECT_EQ(project_->dname(), Project_Entity_Name);
    EXPECT_EQ(project_->parent(), nullptr);
    EXPECT_EQ(static_cast<const Project*>(project_.get())->parent(), nullptr);
    EXPECT_EQ(project_->root(), Test_Root);
    ASSERT_EQ(project_->docs().description().size(), 1);
    EXPECT_EQ(project_->docs().description()[0], Project_Entity_Description);
}

TEST_F(ProjectEntityTest, Ctor_Does_Not_Initialize_Builtins_Api_And_Services)
{
    EXPECT_FALSE(Project().errc());
    EXPECT_FALSE(Project().exception());
    EXPECT_FALSE(Project().callMessage());
    EXPECT_FALSE(Project().resultMessage());
    EXPECT_FALSE(Project().api());
    EXPECT_FALSE(Project().implementation());
}

TEST_F(ProjectEntityTest, Adding_Errc_Enum_Sets_Api_Error_Code_Type)
{
    Enum* errc = nullptr;

    EXPECT_TRUE(errc = project_->addEnum(Errc_Enum_Name, "file.proto"));
    EXPECT_EQ(errc, project_->errc());
}

TEST_F(ProjectEntityTest, Adding_Exception_Struct_Sets_Api_Exception_Type)
{
    Struct* exception = nullptr;
    auto name = GetPredefinedStructName(StructTypeId::Exception);

    EXPECT_TRUE(exception = project_->addStruct(name, "file.proto"));
    EXPECT_EQ(exception, project_->exception());
}

TEST_F(ProjectEntityTest, Adding_CallMessage_Struct_Sets_Api_Call_Message_Type)
{
    Struct* callMsg = nullptr;
    auto name = GetPredefinedStructName(StructTypeId::Call_Message);

    EXPECT_TRUE(callMsg = project_->addStruct(name, Busrpc_Builtin_File));
    EXPECT_EQ(callMsg, project_->callMessage());
}

TEST_F(ProjectEntityTest, Adding_ResultMessage_Struct_Sets_Api_Result_Message_Type)
{
    Struct* resultMsg = nullptr;
    auto name = GetPredefinedStructName(StructTypeId::Result_Message);

    EXPECT_TRUE(resultMsg = project_->addStruct(name, Busrpc_Builtin_File));
    EXPECT_EQ(resultMsg, project_->resultMessage());
}

TEST_F(ProjectEntityTest, addApi_Initializes_Api_Entity)
{
    Project project;
    Api* api = nullptr;

    EXPECT_TRUE(api = project.addApi());
    EXPECT_EQ(api, project.api());
}

TEST_F(ProjectEntityTest, addImplementation_Initializes_Implementation_Entity)
{
    Project project;
    Implementation* implementation = nullptr;

    EXPECT_TRUE(implementation = project.addImplementation());
    EXPECT_EQ(implementation, project.implementation());
}

TEST_F(ProjectEntityTest, find_Returns_Correct_Entity_If_It_Exists)
{
    std::string apiPrefix = JoinStrings(Project_Entity_Name, ".", Api_Entity_Name, ".");
    std::string svcPrefix = JoinStrings(Project_Entity_Name, ".", Implementation_Entity_Name, ".");

    EXPECT_EQ(project_->find(std::string(Project_Entity_Name)), project_.get());
    EXPECT_EQ(project_->find(JoinStrings(Project_Entity_Name, ".", Errc_Enum_Name)), project_->errc());
    EXPECT_EQ(
        project_->find(JoinStrings(Project_Entity_Name, ".", GetPredefinedStructName(StructTypeId::Call_Message))),
        project_->callMessage());

    EXPECT_EQ(project_->find(JoinStrings(Project_Entity_Name, ".", Api_Entity_Name)), api_);
    EXPECT_EQ(project_->find(JoinStrings(Project_Entity_Name, ".", Implementation_Entity_Name)), implementation_);

    EXPECT_EQ(project_->find(apiPrefix + "ns1"), ns1_);
    EXPECT_EQ(project_->find(apiPrefix + "ns2"), ns2_);
    EXPECT_EQ(project_->find(apiPrefix + "ns1.cls1"), cls1_);
    EXPECT_EQ(project_->find(apiPrefix + "ns1.cls1.method1"), method1_);
    EXPECT_EQ(project_->find(apiPrefix + "ns1.cls1.method2"), method2_);
    EXPECT_EQ(project_->find(apiPrefix + "ns1.cls1.method1.Enum1"), enum1_);
    EXPECT_EQ(project_->find(apiPrefix + "ns1.cls1.method1.Enum1.constant1"), constant1_);
    EXPECT_EQ(project_->find(apiPrefix + "ns2.Struct1"), struct1_);
    EXPECT_EQ(project_->find(apiPrefix + "ns2.Struct1.field1"), field1_);
    EXPECT_EQ(project_->find(apiPrefix + "ns2.Struct1.NestedStruct1"), nestedStruct1_);
    EXPECT_EQ(project_->find(apiPrefix + "ns2.Struct1.NestedEnum1"), nestedEnum1_);

    EXPECT_EQ(project_->find(svcPrefix + "Struct1"), implementationStruct1_);
    EXPECT_EQ(project_->find(svcPrefix + "service"), service_);
    EXPECT_EQ(project_->find(svcPrefix + "service.Enum1"), serviceEnum1_);
}

TEST_F(ProjectEntityTest, find_Automatically_Adds_Busprc_Prefix_To_Dname_If_Missing)
{
    EXPECT_EQ(project_->find(""), project_.get());
    EXPECT_EQ(project_->find(Api_Entity_Name), api_);
    EXPECT_EQ(project_->find(std::string(Api_Entity_Name) + ".ns1.cls1.method1"), method1_);
}

TEST_F(ProjectEntityTest, find_Returns_Nullptr_If_Dname_Is_Not_Found)
{
    EXPECT_FALSE(project_->find("unknown"));
    EXPECT_FALSE(project_->find(JoinStrings(Api_Entity_Name, ".unknown")));
    EXPECT_FALSE(project_->find(JoinStrings(Api_Entity_Name, ".ns1.unknown")));
    EXPECT_FALSE(project_->find(JoinStrings(Api_Entity_Name, ".ns1.cls1.unknown")));
    EXPECT_FALSE(project_->find(JoinStrings(Api_Entity_Name, ".ns1.cls1.method1.unknown")));
    EXPECT_FALSE(project_->find(JoinStrings(Api_Entity_Name, ".ns1.cls1.method1.Enum1.unknown")));
    EXPECT_FALSE(project_->find(JoinStrings(Api_Entity_Name, ".ns2.Struct1.unknown")));
    EXPECT_FALSE(project_->find(JoinStrings(Api_Entity_Name, ".ns2.Struct1.unknown")));
    EXPECT_FALSE(project_->find(JoinStrings(Api_Entity_Name, ".ns2.Struct1.NestedStruct1.unknown")));
    EXPECT_FALSE(project_->find(JoinStrings(Api_Entity_Name, ".ns2.Struct1.NestedEnum1.unknown")));
}

TEST_F(ProjectEntityTest, Entity_Directory_Is_Updated_When_New_Entity_Is_Added)
{
    Namespace* ns3 = nullptr;

    EXPECT_FALSE(project_->find(JoinStrings(Api_Entity_Name, ".ns3")));
    EXPECT_TRUE(ns3 = api_->addNamespace("ns3"));
    EXPECT_EQ(project_->find(JoinStrings(Api_Entity_Name, ".ns3")), ns3);
}
}} // namespace busrpc::test
