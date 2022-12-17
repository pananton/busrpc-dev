#include "entities/project.h"

#include <gtest/gtest.h>

namespace busrpc { namespace test {

class TestImportedMethod: public ImportedMethod {
public:
    TestImportedMethod(std::string dname, EntityDocs docs = {}): ImportedMethod(std::move(dname), std::move(docs)) { }
};

class ServiceEntityTest: public ::testing::Test {
protected:
    void SetUp() override
    {
        project_ = std::make_shared<Project>();
        auto services = project_->addServices();

        services_ = services;
        service_ = services->addService("service");
    }

protected:
    std::shared_ptr<Project> project_;
    const Services* services_ = nullptr;
    Service* service_ = nullptr;
};

TEST_F(ServiceEntityTest, Service_Entity_Is_Correctly_Initialized_When_Created_By_Services_Entity)
{
    EXPECT_EQ(service_->type(), EntityTypeId::Service);
    EXPECT_EQ(service_->name(), "service");
    EXPECT_EQ(service_->dir(), std::filesystem::path(Services_Entity_Name) / "service");
    EXPECT_EQ(service_->dname(), std::string(Project_Entity_Name) + "." + Services_Entity_Name + ".service");
    EXPECT_TRUE(service_->docs().description().empty());
    EXPECT_TRUE(service_->docs().brief().empty());
    EXPECT_TRUE(service_->docs().commands().empty());
    EXPECT_EQ(service_->parent(), services_);
    EXPECT_EQ(static_cast<const Service*>(service_)->parent(), services_);
    EXPECT_FALSE(service_->descriptor());
    EXPECT_FALSE(service_->config());
    EXPECT_TRUE(service_->implementedMethods().empty());
    EXPECT_TRUE(service_->invokedMethods().empty());
}

TEST_F(ServiceEntityTest, Adding_ServiceDesc_Struct_Sets_Service_Descriptor)
{
    Struct* desc = nullptr;

    EXPECT_TRUE(desc = service_->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), "service.proto"));
    EXPECT_EQ(desc, service_->descriptor());
}

TEST_F(ServiceEntityTest, Adding_ServiceDesc_Struct_Sets_Service_Documentation)
{
    EntityDocs docs({"Brief description.", "Description."},
                    {{doc_cmd::Service_Author, {"John Doe"}},
                     {doc_cmd::Service_Email, {"jdoe@company.com"}},
                     {doc_cmd::Service_Url, {"git@company.com:project/services/service.git"}}});
    Struct* desc = nullptr;

    EXPECT_TRUE(desc = service_->addStruct(
                    GetPredefinedStructName(StructTypeId::Service_Desc), "service.proto", StructFlags::None, docs));
    EXPECT_EQ(desc, service_->descriptor());
    EXPECT_EQ(service_->docs().description(), docs.description());
    EXPECT_EQ(service_->docs().brief(), docs.brief());
    EXPECT_EQ(service_->docs().commands(), docs.commands());
    EXPECT_EQ(service_->author(), docs.commands().find(doc_cmd::Service_Author)->second.back());
    EXPECT_EQ(service_->email(), docs.commands().find(doc_cmd::Service_Email)->second.back());
    EXPECT_EQ(service_->url(), docs.commands().find(doc_cmd::Service_Url)->second.back());
}

TEST_F(ServiceEntityTest, Adding_Config_Struct_To_Descriptor_Sets_Service_Config)
{
    Struct* desc = nullptr;
    Struct* config = nullptr;

    ASSERT_TRUE(desc = service_->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), "service.proto"));
    EXPECT_TRUE(config = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Config)));
    EXPECT_EQ(service_->config(), config);
}

TEST(ComponentEntityTest, OrderImportedMethodsByDnameAsc_Returns_Correct_Result)
{
    TestImportedMethod m1("method1");
    TestImportedMethod m2("method2");

    EXPECT_TRUE(OrderImportedMethodsByDnameAsc()(m1, m2));
    EXPECT_FALSE(OrderImportedMethodsByDnameAsc()(m2, m1));
    EXPECT_TRUE(OrderImportedMethodsByDnameAsc()(m1, "method2"));
    EXPECT_FALSE(OrderImportedMethodsByDnameAsc()(m2, "method1"));
    EXPECT_TRUE(OrderImportedMethodsByDnameAsc()("method1", m2));
    EXPECT_FALSE(OrderImportedMethodsByDnameAsc()("method2", m1));
}

TEST_F(ServiceEntityTest, Adding_Field_To_Implements_Struct_Creates_Implemented_Method)
{
    Struct* desc = nullptr;
    Struct* impl = nullptr;
    std::string methodName =
        std::string(Project_Entity_Name) + "." + std::string(Api_Entity_Name) + ".namespace.class.method1";

    ASSERT_TRUE(desc = service_->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), "service.proto"));
    ASSERT_TRUE(impl = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Implements)));
    EXPECT_TRUE(service_->implementedMethods().empty());

    EXPECT_TRUE(
        impl->addStructField("field1", 1, methodName + "." + GetPredefinedStructName(StructTypeId::Method_Desc)));
    ASSERT_NE(service_->implementedMethods().find(methodName), service_->implementedMethods().end());
    EXPECT_EQ(service_->implementedMethods().size(), 1);

    const auto& implMethod = *(service_->implementedMethods().find(methodName));

    EXPECT_EQ(implMethod.dname(), methodName);
    EXPECT_TRUE(implMethod.docs().description().empty());
    EXPECT_TRUE(implMethod.docs().brief().empty());
    EXPECT_TRUE(implMethod.docs().commands().empty());
}

TEST_F(ServiceEntityTest, Implemented_Method_Documentation_Is_Correctly_Initialized)
{
    EntityDocs docs(
        {"Brief description.", "Description."},
        {{"cmd", {"cmd value"}}, {doc_cmd::Accepted_Value, {"param1 value1", "@object_id some id", "param2"}}});
    Struct* desc = nullptr;
    Struct* impl = nullptr;
    std::string methodName =
        std::string(Project_Entity_Name) + "." + std::string(Api_Entity_Name) + ".namespace.class.method1";

    ASSERT_TRUE(desc = service_->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), "service.proto"));
    ASSERT_TRUE(impl = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Implements)));
    ASSERT_TRUE(impl->addStructField("field1",
                                     1,
                                     methodName + "." + GetPredefinedStructName(StructTypeId::Method_Desc),
                                     FieldFlags::None,
                                     "",
                                     docs));
    ASSERT_NE(service_->implementedMethods().find(methodName), service_->implementedMethods().end());
    EXPECT_EQ(service_->implementedMethods().size(), 1);

    const auto& implMethod = *(service_->implementedMethods().find(methodName));

    EXPECT_EQ(implMethod.docs().description(), docs.description());
    EXPECT_EQ(implMethod.docs().brief(), docs.brief());
    EXPECT_EQ(implMethod.docs().commands(), docs.commands());
    ASSERT_TRUE(implMethod.acceptedObjectId());
    EXPECT_EQ(*implMethod.acceptedObjectId(), "some id");
    ASSERT_NE(implMethod.acceptedParams().find("param1"), implMethod.acceptedParams().end());
    EXPECT_EQ(implMethod.acceptedParams().find("param1")->second, "value1");
    ASSERT_NE(implMethod.acceptedParams().find("param2"), implMethod.acceptedParams().end());
    EXPECT_EQ(implMethod.acceptedParams().find("param2")->second, "");
    EXPECT_EQ(implMethod.acceptedParams().size(), 2);
}

TEST_F(ServiceEntityTest, Adding_Fields_To_Invokes_Struct_Creates_Invoked_Method)
{
    Struct* desc = nullptr;
    Struct* invk = nullptr;
    std::string methodName =
        std::string(Project_Entity_Name) + "." + std::string(Api_Entity_Name) + ".namespace.class.method1";

    ASSERT_TRUE(desc = service_->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), "service.proto"));
    ASSERT_TRUE(invk = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Invokes)));
    EXPECT_TRUE(service_->invokedMethods().empty());

    EXPECT_TRUE(
        invk->addStructField("field1", 1, methodName + "." + GetPredefinedStructName(StructTypeId::Method_Desc)));
    ASSERT_NE(service_->invokedMethods().find(methodName), service_->invokedMethods().end());
    EXPECT_EQ(service_->invokedMethods().size(), 1);

    const auto& invkMethod = *(service_->invokedMethods().find(methodName));

    EXPECT_EQ(invkMethod.dname(), methodName);
    EXPECT_TRUE(invkMethod.docs().description().empty());
    EXPECT_TRUE(invkMethod.docs().brief().empty());
    EXPECT_TRUE(invkMethod.docs().commands().empty());
}

TEST_F(ServiceEntityTest, Invoked_Method_Documentation_Is_Correctly_Initialized)
{
    EntityDocs docs({"Brief description.", "Description"}, {{"cmd", {"cmd value"}}});
    Struct* desc = nullptr;
    Struct* invk = nullptr;
    std::string methodName =
        std::string(Project_Entity_Name) + "." + std::string(Api_Entity_Name) + ".namespace.class.method1";

    ASSERT_TRUE(desc = service_->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), "service.proto"));
    ASSERT_TRUE(invk = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Invokes)));
    ASSERT_TRUE(invk->addStructField("field1",
                                     1,
                                     methodName + "." + GetPredefinedStructName(StructTypeId::Method_Desc),
                                     FieldFlags::None,
                                     "",
                                     docs));
    ASSERT_NE(service_->invokedMethods().find(methodName), service_->invokedMethods().end());
    EXPECT_EQ(service_->invokedMethods().size(), 1);

    const auto& invkMethod = *(service_->invokedMethods().find(methodName));

    EXPECT_EQ(invkMethod.docs().description(), docs.description());
    EXPECT_EQ(invkMethod.docs().brief(), docs.brief());
    EXPECT_EQ(invkMethod.docs().commands(), docs.commands());
}
}} // namespace busrpc::test
