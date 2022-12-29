#include "entities/project.h"
#include "generators/json_generator.h"
#include "tests_configure.h"
#include "utils/project_utils.h"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <fstream>
#include <sstream>

using json = nlohmann::json;

namespace busrpc { namespace test {

json GetGeneratedJson(const Project& project)
{
    std::ostringstream out;
    out << std::setw(2);
    JsonGenerator generator(out);

    EXPECT_NO_THROW(generator.generate(project));

    json result;

    EXPECT_NO_THROW(result = json::parse(out.str()));

    return result;
}

void TestCommonEntityProperties(const json& obj, const Entity& entity);
void TestGeneralCompositeEntityProperties(const json& obj, const GeneralCompositeEntity& entity);
void TestEnumProperties(const json& obj, const Enum& enumeration);
void TestConstantProperties(const json& obj, const Constant& constant);
void TestStructProperties(const json& obj, const Struct& structure);
void TestFieldProperties(const json& obj, const Field& field);

void TestDocsProperties(const json& obj, const EntityDocs& docs)
{
    ASSERT_FALSE(obj.is_null());
    ASSERT_TRUE(obj.contains("brief"));
    ASSERT_TRUE(obj.contains("description"));
    ASSERT_TRUE(obj.contains("commands"));
    EXPECT_EQ(obj["brief"], docs.brief());
    EXPECT_EQ(obj["description"], docs.description());

    for (const auto& command: docs.commands()) {
        ASSERT_TRUE(obj["commands"].contains(command.first));
        EXPECT_EQ(obj["commands"][command.first], command.second);
    }
}

void TestCommonEntityProperties(const json& obj, const Entity& entity)
{
    ASSERT_FALSE(obj.is_null());
    ASSERT_TRUE(obj.contains("name"));
    ASSERT_TRUE(obj.contains("dname"));
    ASSERT_TRUE(obj.contains("dir"));
    ASSERT_TRUE(obj.contains("docs"));

    EXPECT_EQ(obj["name"], entity.name());
    EXPECT_EQ(obj["dname"], entity.dname());
    EXPECT_EQ(obj["dir"], entity.dir().generic_string());

    TestDocsProperties(obj["docs"], entity.docs());
}

void TestGeneralCompositeEntityProperties(const json& obj, const GeneralCompositeEntity& entity)
{
    TestCommonEntityProperties(obj, entity);
    ASSERT_TRUE(obj.contains("enums"));
    ASSERT_TRUE(obj.contains("structs"));

    size_t expectedCount = 0;

    if (!entity.enums().empty()) {
        for (const auto& enumeration: entity.enums()) {
            if (entity.type() != EntityTypeId::Project || enumeration->name() != Errc_Enum_Name) {
                ASSERT_TRUE(obj["enums"].contains(enumeration->name()));
                TestEnumProperties(obj["enums"][enumeration->name()], *enumeration);
                ++expectedCount;
            }
        }
    }

    if (expectedCount == 0) {
        EXPECT_TRUE(obj["enums"].is_null());
    } else {
        EXPECT_EQ(obj["enums"].size(), expectedCount);
    }

    expectedCount = 0;

    if (!entity.structs().empty()) {
        for (const auto& structure: entity.structs()) {
            if (structure->structType() == StructTypeId::General) {
                ASSERT_TRUE(obj["structs"].contains(structure->name()));
                TestStructProperties(obj["structs"][structure->name()], *structure);
                ++expectedCount;
            }
        }
    }

    if (expectedCount == 0) {
        EXPECT_TRUE(obj["structs"].is_null());
    } else {
        EXPECT_EQ(obj["structs"].size(), expectedCount);
    }
}

void TestEnumProperties(const json& obj, const Enum& enumeration)
{
    TestCommonEntityProperties(obj, enumeration);
    ASSERT_TRUE(obj.contains("package"));
    ASSERT_TRUE(obj.contains("file"));

    EXPECT_EQ(obj["package"], enumeration.package());
    EXPECT_EQ(obj["file"], enumeration.file().generic_string());

    if (!enumeration.constants().empty()) {
        for (const auto& constant: enumeration.constants()) {
            ASSERT_TRUE(obj["constants"].contains(constant->name()));
            TestConstantProperties(obj["constants"][constant->name()], *constant);
        }
    } else {
        EXPECT_TRUE(obj["constants"].is_null());
    }
}

void TestConstantProperties(const json& obj, const Constant& constant)
{
    TestCommonEntityProperties(obj, constant);
    ASSERT_TRUE(obj.contains("value"));
    EXPECT_EQ(obj["value"], constant.value());
}

void TestStructProperties(const json& obj, const Struct& structure)
{
    TestGeneralCompositeEntityProperties(obj, structure);
    ASSERT_TRUE(obj.contains("package"));
    ASSERT_TRUE(obj.contains("file"));
    ASSERT_TRUE(obj.contains("isHashed"));
    ASSERT_TRUE(obj.contains("isEncodable"));

    EXPECT_EQ(obj["package"], structure.package());
    EXPECT_EQ(obj["file"], structure.file().generic_string());
    EXPECT_EQ(obj["isHashed"], structure.isHashed());
    EXPECT_EQ(obj["isEncodable"], structure.isEncodable());

    if (!structure.fields().empty()) {
        for (const auto& field: structure.fields()) {
            ASSERT_TRUE(obj["fields"].contains(field->name()));
            TestFieldProperties(obj["fields"][field->name()], *field);
        }
    } else {
        EXPECT_TRUE(obj["fields"].is_null());
    }
}

void TestFieldProperties(const json& obj, const Field& field)
{
    TestCommonEntityProperties(obj, field);
    ASSERT_TRUE(obj.contains("number"));
    ASSERT_TRUE(obj.contains("fieldTypeName"));
    ASSERT_TRUE(obj.contains("isOptional"));
    ASSERT_TRUE(obj.contains("isRepeated"));
    ASSERT_TRUE(obj.contains("isObservable"));
    ASSERT_TRUE(obj.contains("isHashed"));
    ASSERT_TRUE(obj.contains("oneofName"));
    ASSERT_TRUE(obj.contains("defaultValue"));
    ASSERT_TRUE(obj.contains("isMap"));

    if (field.fieldType() == FieldTypeId::Map) {
        ASSERT_TRUE(obj.contains("keyTypeName"));
        ASSERT_TRUE(obj.contains("valueTypeName"));
    }

    EXPECT_EQ(obj["number"], field.number());
    EXPECT_EQ(obj["fieldTypeName"], field.fieldTypeName());
    EXPECT_EQ(obj["isOptional"], field.isOptional());
    EXPECT_EQ(obj["isRepeated"], field.isRepeated());
    EXPECT_EQ(obj["isObservable"], field.isObservable());
    EXPECT_EQ(obj["isHashed"], field.isHashed());
    EXPECT_EQ(obj["oneofName"], field.oneofName());
    EXPECT_EQ(obj["defaultValue"], field.defaultValue());
    EXPECT_EQ(obj["isMap"], field.fieldType() == FieldTypeId::Map);

    if (field.fieldType() == FieldTypeId::Map) {
        EXPECT_EQ(obj["keyTypeName"], static_cast<const MapField&>(field).keyTypeName());
        EXPECT_EQ(obj["valueTypeName"], static_cast<const MapField&>(field).valueTypeName());
    }
}

TEST(JsonGeneratorTest, Expected_Documentation_Is_Generated_For_Empty_Project)
{
    Project project;
    auto jsonProject = GetGeneratedJson(project);

    TestGeneralCompositeEntityProperties(jsonProject, project);

    ASSERT_TRUE(jsonProject.contains(Errc_Enum_Name));
    ASSERT_TRUE(jsonProject.contains(GetPredefinedStructName(StructTypeId::Exception)));
    ASSERT_TRUE(jsonProject.contains(GetPredefinedStructName(StructTypeId::Call_Message)));
    ASSERT_TRUE(jsonProject.contains(GetPredefinedStructName(StructTypeId::Result_Message)));
    ASSERT_TRUE(jsonProject.contains("api"));
    ASSERT_TRUE(jsonProject.contains("implementation"));

    EXPECT_TRUE(jsonProject[Errc_Enum_Name].is_null());
    EXPECT_TRUE(jsonProject[GetPredefinedStructName(StructTypeId::Exception)].is_null());
    EXPECT_TRUE(jsonProject[GetPredefinedStructName(StructTypeId::Call_Message)].is_null());
    EXPECT_TRUE(jsonProject[GetPredefinedStructName(StructTypeId::Result_Message)].is_null());

    EXPECT_TRUE(jsonProject["api"].is_null());
    EXPECT_TRUE(jsonProject["implementation"].is_null());
}

TEST(JsonGeneratorTest,
     Expected_Documentation_Is_Generated_For_Project_With_Builtins_Api_Implementation_Structs_And_Enums)
{
    Project project;
    InitMinimalProject(&project);
    project.addApi();
    project.addImplementation();
    auto jsonProject = GetGeneratedJson(project);

    TestGeneralCompositeEntityProperties(jsonProject, project);

    ASSERT_TRUE(jsonProject.contains(Errc_Enum_Name));
    ASSERT_TRUE(jsonProject.contains(GetPredefinedStructName(StructTypeId::Exception)));
    ASSERT_TRUE(jsonProject.contains(GetPredefinedStructName(StructTypeId::Call_Message)));
    ASSERT_TRUE(jsonProject.contains(GetPredefinedStructName(StructTypeId::Result_Message)));
    ASSERT_TRUE(jsonProject.contains("api"));
    ASSERT_TRUE(jsonProject.contains("implementation"));

    TestEnumProperties(jsonProject[Errc_Enum_Name], *project.errc());
    TestStructProperties(jsonProject[GetPredefinedStructName(StructTypeId::Exception)], *project.exception());
    TestStructProperties(jsonProject[GetPredefinedStructName(StructTypeId::Call_Message)], *project.callMessage());
    TestStructProperties(jsonProject[GetPredefinedStructName(StructTypeId::Result_Message)], *project.resultMessage());

    EXPECT_FALSE(jsonProject["api"].is_null());
    EXPECT_FALSE(jsonProject["implementation"].is_null());
}

TEST(JsonGeneratorTest, Expected_Documentation_Is_Generated_For_Empty_Api)
{
    Project project;
    auto api = project.addApi();
    json jsonApi = GetGeneratedJson(project)["api"];

    ASSERT_TRUE(jsonApi.contains("namespaces"));
    EXPECT_TRUE(jsonApi["namespaces"].is_null());

    TestGeneralCompositeEntityProperties(jsonApi, *api);
}

TEST(JsonGeneratorTest, Expected_Documentation_Is_Generated_For_Api_With_Namespace_Structs_And_Enums)
{
    Project project;
    auto api = AddApi(&project);
    api->addNamespace("namespace");
    json jsonApi = GetGeneratedJson(project)["api"];

    ASSERT_TRUE(jsonApi.contains("namespaces"));
    EXPECT_TRUE(jsonApi["namespaces"].contains("namespace"));
    EXPECT_EQ(jsonApi["namespaces"].size(), 1);

    TestGeneralCompositeEntityProperties(jsonApi, *api);
}

TEST(JsonGeneratorTest, Expected_Documentation_Is_Generated_For_Empty_Namespace)
{
    Project project;
    auto ns = AddApi(&project)->addNamespace("namespace");
    json jsonNamespace = GetGeneratedJson(project)["api"]["namespaces"]["namespace"];

    ASSERT_TRUE(jsonNamespace.contains("classes"));
    EXPECT_TRUE(jsonNamespace["classes"].is_null());

    TestGeneralCompositeEntityProperties(jsonNamespace, *ns);
}

TEST(JsonGeneratorTest, Expected_Documentation_Is_Generated_For_Namespace_With_Class_Structs_And_Enums)
{
    Project project;
    auto ns = AddNamespace(AddApi(&project));
    ns->addClass("class");
    json jsonNamespace = GetGeneratedJson(project)["api"]["namespaces"]["namespace"];

    ASSERT_TRUE(jsonNamespace.contains("classes"));
    ASSERT_TRUE(jsonNamespace["classes"].contains("class"));
    EXPECT_EQ(jsonNamespace["classes"].size(), 1);

    TestGeneralCompositeEntityProperties(jsonNamespace, *ns);
}

TEST(JsonGeneratorTest, Expected_Documentation_Is_Generated_For_Empty_Class)
{
    Project project;
    auto cls = AddNamespace(AddApi(&project))->addClass("class");
    json jsonClass = GetGeneratedJson(project)["api"]["namespaces"]["namespace"]["classes"]["class"];

    ASSERT_TRUE(jsonClass.contains(GetPredefinedStructName(StructTypeId::Class_Object_Id)));
    ASSERT_TRUE(jsonClass.contains("isStatic"));
    ASSERT_TRUE(jsonClass.contains("methods"));
    EXPECT_TRUE(jsonClass[GetPredefinedStructName(StructTypeId::Class_Object_Id)].is_null());
    EXPECT_TRUE(jsonClass["isStatic"]);
    EXPECT_TRUE(jsonClass["methods"].is_null());

    TestGeneralCompositeEntityProperties(jsonClass, *cls);
}

TEST(JsonGeneratorTest, Expected_Documentation_Is_Generated_For_Class_With_Method_Structs_And_Enums)
{
    Project project;
    auto cls = AddClass(AddNamespace(AddApi(&project)));
    cls->addMethod("method");
    json jsonClass = GetGeneratedJson(project)["api"]["namespaces"]["namespace"]["classes"]["class"];

    ASSERT_TRUE(jsonClass.contains(GetPredefinedStructName(StructTypeId::Class_Object_Id)));
    ASSERT_TRUE(jsonClass.contains("isStatic"));
    ASSERT_TRUE(jsonClass.contains("methods"));
    EXPECT_FALSE(jsonClass["isStatic"]);
    EXPECT_EQ(jsonClass["methods"].size(), 1);

    TestGeneralCompositeEntityProperties(jsonClass[GetPredefinedStructName(StructTypeId::Class_Object_Id)],
                                         *cls->objectId());
    TestGeneralCompositeEntityProperties(jsonClass, *cls);
}

TEST(JsonGeneratorTest, Expected_Documentation_Is_Generated_For_Static_Class_With_Method_Structs_And_Enums)
{
    Project project;
    auto cls = AddClass(AddNamespace(AddApi(&project)), true);
    cls->addMethod("method");
    json jsonClass = GetGeneratedJson(project)["api"]["namespaces"]["namespace"]["classes"]["class"];

    ASSERT_TRUE(jsonClass.contains(GetPredefinedStructName(StructTypeId::Class_Object_Id)));
    ASSERT_TRUE(jsonClass.contains("isStatic"));
    ASSERT_TRUE(jsonClass.contains("methods"));
    EXPECT_TRUE(jsonClass[GetPredefinedStructName(StructTypeId::Class_Object_Id)].is_null());
    EXPECT_TRUE(jsonClass["isStatic"]);
    EXPECT_EQ(jsonClass["methods"].size(), 1);

    TestGeneralCompositeEntityProperties(jsonClass, *cls);
}

TEST(JsonGeneratorTest, Expected_Documentation_Is_Generated_For_Empty_Method)
{
    Project project;
    auto method = AddClass(AddNamespace(AddApi(&project)))->addMethod("method");
    json jsonMethod =
        GetGeneratedJson(project)["api"]["namespaces"]["namespace"]["classes"]["class"]["methods"]["method"];

    ASSERT_TRUE(jsonMethod.contains(GetPredefinedStructName(StructTypeId::Method_Params)));
    ASSERT_TRUE(jsonMethod.contains(GetPredefinedStructName(StructTypeId::Method_Retval)));
    ASSERT_TRUE(jsonMethod.contains("isStatic"));
    ASSERT_TRUE(jsonMethod.contains("isOneway"));
    ASSERT_TRUE(jsonMethod.contains("precondition"));
    ASSERT_TRUE(jsonMethod.contains("postcondition"));
    EXPECT_TRUE(jsonMethod[GetPredefinedStructName(StructTypeId::Method_Params)].is_null());
    EXPECT_TRUE(jsonMethod[GetPredefinedStructName(StructTypeId::Method_Retval)].is_null());
    EXPECT_FALSE(jsonMethod["isStatic"]);
    EXPECT_TRUE(jsonMethod["isOneway"]);
    EXPECT_EQ(jsonMethod["precondition"], "");
    EXPECT_EQ(jsonMethod["postcondition"], "");

    TestGeneralCompositeEntityProperties(jsonMethod, *method);
}

TEST(JsonGeneratorTest, Expected_Documentation_Is_Generated_For_Method_With_Structs_And_Enums)
{
    Project project;
    auto method = AddMethod(AddClass(AddNamespace(AddApi(&project))));
    json jsonMethod =
        GetGeneratedJson(project)["api"]["namespaces"]["namespace"]["classes"]["class"]["methods"]["method"];

    ASSERT_TRUE(jsonMethod.contains(GetPredefinedStructName(StructTypeId::Method_Params)));
    ASSERT_TRUE(jsonMethod.contains(GetPredefinedStructName(StructTypeId::Method_Retval)));
    ASSERT_TRUE(jsonMethod.contains("isStatic"));
    ASSERT_TRUE(jsonMethod.contains("isOneway"));
    ASSERT_TRUE(jsonMethod.contains("precondition"));
    ASSERT_TRUE(jsonMethod.contains("postcondition"));
    EXPECT_FALSE(jsonMethod["isStatic"]);
    EXPECT_FALSE(jsonMethod["isOneway"]);
    EXPECT_EQ(jsonMethod["precondition"], "precondition");
    EXPECT_EQ(jsonMethod["postcondition"], "postcondition");

    TestGeneralCompositeEntityProperties(jsonMethod[GetPredefinedStructName(StructTypeId::Method_Params)],
                                         *(method->params()));
    TestGeneralCompositeEntityProperties(jsonMethod[GetPredefinedStructName(StructTypeId::Method_Retval)],
                                         *(method->retval()));
    TestGeneralCompositeEntityProperties(jsonMethod, *method);
}

TEST(JsonGeneratorTest,
     Expected_Documentation_Is_Generated_For_Static_Method_Wo_Params_And_Retval_And_With_Structs_And_Enums)
{
    Project project;
    auto method = AddMethod(AddClass(AddNamespace(AddApi(&project))), true, false, false);
    json jsonMethod =
        GetGeneratedJson(project)["api"]["namespaces"]["namespace"]["classes"]["class"]["methods"]["method"];

    ASSERT_TRUE(jsonMethod.contains(GetPredefinedStructName(StructTypeId::Method_Params)));
    ASSERT_TRUE(jsonMethod.contains(GetPredefinedStructName(StructTypeId::Method_Retval)));
    ASSERT_TRUE(jsonMethod.contains("isStatic"));
    ASSERT_TRUE(jsonMethod.contains("isOneway"));
    ASSERT_TRUE(jsonMethod.contains("precondition"));
    ASSERT_TRUE(jsonMethod.contains("postcondition"));
    EXPECT_TRUE(jsonMethod["params"].is_null());
    EXPECT_TRUE(jsonMethod[GetPredefinedStructName(StructTypeId::Method_Params)].is_null());
    EXPECT_TRUE(jsonMethod[GetPredefinedStructName(StructTypeId::Method_Retval)].is_null());
    EXPECT_TRUE(jsonMethod["isStatic"]);
    EXPECT_TRUE(jsonMethod["isOneway"]);
    EXPECT_EQ(jsonMethod["precondition"], method->precondition());
    EXPECT_EQ(jsonMethod["postcondition"], method->postcondition());

    TestGeneralCompositeEntityProperties(jsonMethod, *method);
}

TEST(JsonGeneratorTest, Expected_Documentation_Is_Generated_For_Empty_Implementation)
{
    Project project;
    auto impl = project.addImplementation();
    json jsonImpl = GetGeneratedJson(project)["implementation"];

    ASSERT_TRUE(jsonImpl.contains("services"));
    EXPECT_TRUE(jsonImpl["services"].is_null());

    TestGeneralCompositeEntityProperties(jsonImpl, *impl);
}

TEST(JsonGeneratorTest, Expected_Documentation_Is_Generated_For_Implementation_With_Service_Structs_And_Enums)
{
    Project project;
    auto impl = AddImplementation(&project);
    impl->addService("service");
    json jsonImpl = GetGeneratedJson(project)["implementation"];

    ASSERT_TRUE(jsonImpl.contains("services"));
    EXPECT_TRUE(jsonImpl["services"].contains("service"));
    EXPECT_EQ(jsonImpl["services"].size(), 1);

    TestGeneralCompositeEntityProperties(jsonImpl, *impl);
}

TEST(JsonGeneratorTest, Expected_Documentation_Is_Generated_For_Empty_Service)
{
    Project project;
    auto service = AddImplementation(&project)->addService("service");
    json jsonService = GetGeneratedJson(project)["implementation"]["services"]["service"];

    ASSERT_TRUE(jsonService.contains(GetPredefinedStructName(StructTypeId::Service_Config)));
    ASSERT_TRUE(jsonService.contains("implements"));
    ASSERT_TRUE(jsonService.contains("invokes"));
    ASSERT_TRUE(jsonService.contains("author"));
    ASSERT_TRUE(jsonService.contains("email"));
    ASSERT_TRUE(jsonService.contains("url"));
    EXPECT_TRUE(jsonService[GetPredefinedStructName(StructTypeId::Service_Config)].is_null());
    EXPECT_TRUE(jsonService["implements"].is_null());
    EXPECT_TRUE(jsonService["invokes"].is_null());
    EXPECT_EQ(jsonService["author"], "");
    EXPECT_EQ(jsonService["email"], "");
    EXPECT_EQ(jsonService["url"], "");

    TestGeneralCompositeEntityProperties(jsonService, *service);
}

TEST(JsonGeneratorTest, Expected_Documentation_Is_Generated_For_Service_With_Structs_And_Enums)
{
    Project project;
    auto service = AddService(AddImplementation(&project));
    json jsonService = GetGeneratedJson(project)["implementation"]["services"]["service"];

    ASSERT_TRUE(jsonService.contains(GetPredefinedStructName(StructTypeId::Service_Config)));
    ASSERT_TRUE(jsonService.contains("implements"));
    ASSERT_TRUE(jsonService.contains("invokes"));
    ASSERT_TRUE(jsonService.contains("author"));
    ASSERT_TRUE(jsonService.contains("email"));
    ASSERT_TRUE(jsonService.contains("url"));
    EXPECT_EQ(jsonService["author"], service->author());
    EXPECT_EQ(jsonService["email"], service->email());
    EXPECT_EQ(jsonService["url"], service->url());

    const auto& jsonImplements = jsonService["implements"];

    for (const auto& implMethod: service->implementedMethods()) {
        ASSERT_TRUE(jsonImplements.contains(implMethod.dname()));

        const auto& jsonImplMethod = jsonImplements[implMethod.dname()];

        if (implMethod.acceptedObjectId()) {
            ASSERT_TRUE(jsonImplMethod.contains("acceptedObjectId"));
            EXPECT_EQ(jsonImplMethod["acceptedObjectId"], *implMethod.acceptedObjectId());
        } else {
            ASSERT_FALSE(jsonImplMethod.contains("acceptedObjectId"));
        }

        if (!implMethod.acceptedParams().empty()) {
            ASSERT_TRUE(jsonImplMethod.contains("acceptedParams"));

            for (const auto& param: implMethod.acceptedParams()) {
                ASSERT_TRUE(jsonImplMethod["acceptedParams"].contains(param.first));
                EXPECT_EQ(jsonImplMethod["acceptedParams"][param.first], param.second);
            }
        }

        ASSERT_TRUE(jsonImplMethod.contains("docs"));
        TestDocsProperties(jsonImplMethod["docs"], implMethod.docs());
    }

    const auto& jsonInvokes = jsonService["invokes"];

    for (const auto& invkMethod: service->invokedMethods()) {
        ASSERT_TRUE(jsonInvokes.contains(invkMethod.dname()));

        const auto& jsonInvkMethod = jsonInvokes[invkMethod.dname()];

        ASSERT_TRUE(jsonInvkMethod.contains("docs"));
        TestDocsProperties(jsonInvkMethod["docs"], invkMethod.docs());
    }

    TestGeneralCompositeEntityProperties(jsonService[GetPredefinedStructName(StructTypeId::Service_Config)],
                                         *(service->config()));
    TestGeneralCompositeEntityProperties(jsonService, *service);
}

TEST(JsonGeneratorTest,
     Expected_Documentation_Is_Generated_For_Service_Wo_Config_Implements_Invokes_And_With_Structs_And_Enums)
{
    Project project;
    auto service = AddService(AddImplementation(&project), false, false, false);
    json jsonService = GetGeneratedJson(project)["implementation"]["services"]["service"];

    ASSERT_TRUE(jsonService.contains(GetPredefinedStructName(StructTypeId::Service_Config)));
    ASSERT_TRUE(jsonService.contains("implements"));
    ASSERT_TRUE(jsonService.contains("invokes"));
    ASSERT_TRUE(jsonService.contains("author"));
    ASSERT_TRUE(jsonService.contains("email"));
    ASSERT_TRUE(jsonService.contains("url"));
    EXPECT_TRUE(jsonService[GetPredefinedStructName(StructTypeId::Service_Config)].is_null());
    EXPECT_TRUE(jsonService["implements"].is_null());
    EXPECT_TRUE(jsonService["invokes"].is_null());
    EXPECT_EQ(jsonService["author"], service->author());
    EXPECT_EQ(jsonService["email"], service->email());
    EXPECT_EQ(jsonService["url"], service->url());

    TestGeneralCompositeEntityProperties(jsonService, *service);
}
}} // namespace busrpc::test
