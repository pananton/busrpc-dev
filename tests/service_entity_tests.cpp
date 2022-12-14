#include "entities/project.h"

#include <gtest/gtest.h>

namespace busrpc { namespace test {

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
    EXPECT_TRUE(service_->description().empty());
    EXPECT_TRUE(service_->briefDescription().empty());
    EXPECT_TRUE(service_->docCommands().empty());
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
    std::string blockComment = "\\author John Doe\n"
                               "\\email jdoe@company.com\n"
                               "\\url git@company.com:project/services/service.git\n"
                               "Brief description.\n"
                               "Description.";
    Struct* desc = nullptr;

    EXPECT_TRUE(
        desc = service_->addStruct(
            GetPredefinedStructName(StructTypeId::Service_Desc), "service.proto", StructFlags::None, blockComment));
    EXPECT_EQ(desc, service_->descriptor());
    ASSERT_EQ(service_->description().size(), 2);
    EXPECT_EQ(service_->description()[0], "Brief description.");
    EXPECT_EQ(service_->description()[1], "Description.");
    EXPECT_EQ(service_->briefDescription(), "Brief description.");
    EXPECT_EQ(service_->docCommands().size(), 3);
    ASSERT_NE(service_->docCommands().find(Service::Author_Doc_Command), service_->docCommands().end());
    ASSERT_NE(service_->docCommands().find(Service::Email_Doc_Command), service_->docCommands().end());
    ASSERT_NE(service_->docCommands().find(Service::Url_Doc_Command), service_->docCommands().end());
    EXPECT_EQ(service_->docCommands().find(Service::Author_Doc_Command)->second, service_->author());
    EXPECT_EQ(service_->docCommands().find(Service::Email_Doc_Command)->second, service_->email());
    EXPECT_EQ(service_->docCommands().find(Service::Url_Doc_Command)->second, service_->url());
}

TEST_F(ServiceEntityTest, Adding_Config_Struct_To_Descriptor_Sets_Service_Config)
{
    Struct* desc = nullptr;
    Struct* config = nullptr;

    ASSERT_TRUE(desc = service_->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), "service.proto"));
    EXPECT_TRUE(config = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Config)));
    EXPECT_EQ(service_->config(), config);
}

TEST_F(ServiceEntityTest, Adding_Field_To_Implements_Struct_Creates_Implemented_Method)
{
    Struct* desc = nullptr;
    Struct* impl = nullptr;
    std::string methodName = "api.namespace.class.method1";

    ASSERT_TRUE(desc = service_->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), "service.proto"));
    ASSERT_TRUE(impl = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Implements)));
    EXPECT_TRUE(service_->implementedMethods().empty());

    EXPECT_TRUE(
        impl->addStructField("field1", 1, methodName + "." + GetPredefinedStructName(StructTypeId::Method_Desc)));
    ASSERT_NE(service_->implementedMethods().find(methodName), service_->implementedMethods().end());
    EXPECT_EQ(service_->implementedMethods().size(), 1);
    EXPECT_NE(service_->nested().find(methodName), service_->nested().end());

    auto implMethod = service_->implementedMethods().find(methodName)->second;

    EXPECT_EQ(implMethod->type(), EntityTypeId::Implemented_Method);
    EXPECT_EQ(implMethod->name(), methodName);
    EXPECT_EQ(implMethod->dir(), service_->dir());
    EXPECT_EQ(implMethod->parent(), service_);
    EXPECT_TRUE(implMethod->description().empty());
    EXPECT_TRUE(implMethod->briefDescription().empty());
    EXPECT_TRUE(implMethod->docCommands().empty());
}

TEST_F(ServiceEntityTest, Implemented_Method_Documentation_Is_Correctly_Initialized)
{
    Struct* desc = nullptr;
    Struct* impl = nullptr;
    std::string methodName = "api.namespace.class.method";
    std::string blockComment = "\\cmd cmd value\n"
                               "\\accept param1 value1\n"
                               "\\accept @object_id some id\n"
                               "\\accept param2\n"
                               "Brief description.\n"
                               "Description.";

    ASSERT_TRUE(desc = service_->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), "service.proto"));
    ASSERT_TRUE(impl = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Implements)));
    ASSERT_TRUE(impl->addStructField("field1",
                                     1,
                                     methodName + "." + GetPredefinedStructName(StructTypeId::Method_Desc),
                                     FieldFlags::None,
                                     "",
                                     blockComment));
    ASSERT_NE(service_->implementedMethods().find(methodName), service_->implementedMethods().end());
    EXPECT_EQ(service_->implementedMethods().size(), 1);

    auto implMethod = service_->implementedMethods().find(methodName)->second;

    ASSERT_EQ(implMethod->description().size(), 2);
    EXPECT_EQ(implMethod->description()[0], "Brief description.");
    EXPECT_EQ(implMethod->description()[1], "Description.");
    EXPECT_EQ(implMethod->briefDescription(), "Brief description.");
    EXPECT_EQ(implMethod->docCommands().size(), 4);
    ASSERT_NE(implMethod->docCommands().find("cmd"), implMethod->docCommands().end());
    EXPECT_EQ(implMethod->docCommands().find("cmd")->second, "cmd value");

    ASSERT_TRUE(implMethod->acceptedObjectId());
    EXPECT_EQ(*implMethod->acceptedObjectId(), "some id");
    ASSERT_NE(implMethod->acceptedParams().find("param1"), implMethod->acceptedParams().end());
    EXPECT_EQ(implMethod->acceptedParams().find("param1")->second, "value1");
    ASSERT_NE(implMethod->acceptedParams().find("param2"), implMethod->acceptedParams().end());
    EXPECT_EQ(implMethod->acceptedParams().find("param2")->second, "");
    EXPECT_EQ(implMethod->acceptedParams().size(), 2);
}

TEST_F(ServiceEntityTest, Adding_Fields_To_Invokes_Struct_Creates_Invoked_Method)
{
    Struct* desc = nullptr;
    Struct* invk = nullptr;
    std::string methodName = "api.namespace.class.method1";

    ASSERT_TRUE(desc = service_->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), "service.proto"));
    ASSERT_TRUE(invk = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Invokes)));
    EXPECT_TRUE(service_->invokedMethods().empty());

    EXPECT_TRUE(
        invk->addStructField("field1", 1, methodName + "." + GetPredefinedStructName(StructTypeId::Method_Desc)));
    ASSERT_NE(service_->invokedMethods().find(methodName), service_->invokedMethods().end());
    EXPECT_EQ(service_->invokedMethods().size(), 1);
    EXPECT_NE(service_->nested().find(methodName), service_->nested().end());

    auto invkMethod = service_->invokedMethods().find(methodName)->second;

    EXPECT_EQ(invkMethod->type(), EntityTypeId::Implemented_Method);
    EXPECT_EQ(invkMethod->name(), methodName);
    EXPECT_EQ(invkMethod->dir(), service_->dir());
    EXPECT_EQ(invkMethod->parent(), service_);
    EXPECT_TRUE(invkMethod->description().empty());
    EXPECT_TRUE(invkMethod->briefDescription().empty());
    EXPECT_TRUE(invkMethod->docCommands().empty());
}

TEST_F(ServiceEntityTest, Invoked_Method_Documentation_Is_Correctly_Initialized)
{
    Struct* desc = nullptr;
    Struct* invk = nullptr;
    std::string methodName = "api.namespace.class.method";
    std::string blockComment = "\\cmd cmd value\n"
                               "Brief description.\n"
                               "Description.";

    ASSERT_TRUE(desc = service_->addStruct(GetPredefinedStructName(StructTypeId::Service_Desc), "service.proto"));
    ASSERT_TRUE(invk = desc->addStruct(GetPredefinedStructName(StructTypeId::Service_Invokes)));
    ASSERT_TRUE(invk->addStructField("field1",
                                     1,
                                     methodName + "." + GetPredefinedStructName(StructTypeId::Method_Desc),
                                     FieldFlags::None,
                                     "",
                                     blockComment));
    ASSERT_NE(service_->invokedMethods().find(methodName), service_->invokedMethods().end());
    EXPECT_EQ(service_->invokedMethods().size(), 1);

    auto invkMethod = service_->invokedMethods().find(methodName)->second;

    ASSERT_EQ(invkMethod->description().size(), 2);
    EXPECT_EQ(invkMethod->description()[0], "Brief description.");
    EXPECT_EQ(invkMethod->description()[1], "Description.");
    EXPECT_EQ(invkMethod->briefDescription(), "Brief description.");
    EXPECT_EQ(invkMethod->docCommands().size(), 1);
    ASSERT_NE(invkMethod->docCommands().find("cmd"), invkMethod->docCommands().end());
    EXPECT_EQ(invkMethod->docCommands().find("cmd")->second, "cmd value");
}
}} // namespace busrpc::test
