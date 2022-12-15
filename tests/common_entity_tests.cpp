#include "entities/entity.h"
#include "entities/project.h"
#include "utils/common.h"

#include <gtest/gtest.h>

#include <array>

namespace busrpc { namespace test {

class TestEntity: public Entity {
public:
    TestEntity(CompositeEntity* parent, EntityTypeId type, const std::string& name, EntityDocs docs = {}):
        Entity(parent, type, name, std::move(docs))
    { }
};

class TestDistinguishedEntity: public DistinguishedEntity {
public:
    TestDistinguishedEntity(CompositeEntity* parent, EntityTypeId type, const std::string& name):
        DistinguishedEntity(parent, type, name, {})
    { }
};

class TestCompositeEntity: public CompositeEntity {
public:
    TestCompositeEntity(CompositeEntity* parent, EntityTypeId type, const std::string& name):
        CompositeEntity(parent, type, name, {})
    { }

    TestCompositeEntity(CompositeEntity* parent,
                        EntityTypeId type,
                        const std::string& name,
                        std::function<void(Entity*)> onNestedEntityAdded):
        CompositeEntity(parent, type, name, {})
    {
        setNestedEntityAddedCallback(std::move(onNestedEntityAdded));
    }

    using CompositeEntity::addNestedEntity;
};

TEST(CommonEntityTest, GetEntityTypeStr_Returns_Non_Nullptr_For_Known_Entity_Type)
{
    EXPECT_TRUE(GetEntityTypeIdStr(EntityTypeId::Project));
    EXPECT_TRUE(GetEntityTypeIdStr(EntityTypeId::Api));
    EXPECT_TRUE(GetEntityTypeIdStr(EntityTypeId::Services));
    EXPECT_TRUE(GetEntityTypeIdStr(EntityTypeId::Namespace));
    EXPECT_TRUE(GetEntityTypeIdStr(EntityTypeId::Class));
    EXPECT_TRUE(GetEntityTypeIdStr(EntityTypeId::Method));
    EXPECT_TRUE(GetEntityTypeIdStr(EntityTypeId::Struct));
    EXPECT_TRUE(GetEntityTypeIdStr(EntityTypeId::Field));
    EXPECT_TRUE(GetEntityTypeIdStr(EntityTypeId::Enum));
    EXPECT_TRUE(GetEntityTypeIdStr(EntityTypeId::Constant));
    EXPECT_TRUE(GetEntityTypeIdStr(EntityTypeId::Service));
    EXPECT_TRUE(GetEntityTypeIdStr(EntityTypeId::Implemented_Method));
    EXPECT_TRUE(GetEntityTypeIdStr(EntityTypeId::Invoked_Method));
}

TEST(CommonEntityTest, GetEntityTypeStr_Returns_Nullptr_For_Unknown_Entity_Type)
{
    EXPECT_FALSE(GetEntityTypeIdStr(static_cast<EntityTypeId>(0)));
}

TEST(CommonEntityTest, Entity_Ctor_Correctly_Initializes_Object)
{
    std::string entityName = "entity";
    TestEntity entity(nullptr, EntityTypeId::Struct, entityName);

    EXPECT_EQ(entity.type(), EntityTypeId::Struct);
    EXPECT_EQ(entity.name(), entityName);
    EXPECT_TRUE(entity.dir().empty());
    EXPECT_TRUE(entity.docs().description().empty());
    EXPECT_TRUE(entity.docs().brief().empty());
    EXPECT_TRUE(entity.docs().commands().empty());
    EXPECT_FALSE(entity.parent());
    EXPECT_FALSE(static_cast<const Entity*>(&entity)->parent());
}

TEST(CommonEntityTest, Entity_Ctor_Correctly_Initializes_Parent)
{
    {
        TestCompositeEntity parent(nullptr, EntityTypeId::Project, "project");
        TestEntity entity(&parent, EntityTypeId::Api, "api");

        EXPECT_EQ(entity.dir(), "api");
        EXPECT_EQ(entity.parent(), &parent);
        EXPECT_EQ(static_cast<const Entity*>(&entity)->parent(), &parent);
    }

    {
        TestCompositeEntity root(nullptr, EntityTypeId::Project, "project");
        TestCompositeEntity parent(&root, EntityTypeId::Api, "api");

        // next entities should not add new level in the directory hierarchy
        std::array<EntityTypeId, 6> ids = {EntityTypeId::Struct,
                                           EntityTypeId::Field,
                                           EntityTypeId::Enum,
                                           EntityTypeId::Constant,
                                           EntityTypeId::Implemented_Method,
                                           EntityTypeId::Invoked_Method};

        for (auto id: ids) {
            TestEntity entity(&parent, id, "entity");

            EXPECT_EQ(entity.dir(), "api");
        }
    }
}

TEST(CommonEntityTest, Entity_Ctor_Correctly_Initializes_Entity_Documentation)
{
    EntityDocs docs({"Brief description.", "Description."}, {{"cmd1", {"value1"}}, {"cmd2", {"value2", "value3"}}});
    TestEntity entity(nullptr, EntityTypeId::Struct, "entity", docs);

    EXPECT_EQ(entity.docs().description(), docs.description());
    EXPECT_EQ(entity.docs().brief(), docs.brief());
    EXPECT_EQ(entity.docs().commands(), docs.commands());
}

TEST(CommonEntityTest, Distinguished_Entity_Ctor_Correctly_Initializes_Dname)
{
    {
        TestDistinguishedEntity entity(nullptr, EntityTypeId::Project, "project");

        EXPECT_EQ(entity.dname(), "project");
    }

    {
        TestCompositeEntity parent(nullptr, EntityTypeId::Project, "project");
        TestDistinguishedEntity entity(&parent, EntityTypeId::Api, "api");

        EXPECT_EQ(entity.dname(), "project.api");
    }
}

TEST(CommonEntityTest, Composite_Entity_Stores_Added_Nested_Entities)
{
    TestCompositeEntity parent(nullptr, EntityTypeId::Project, "project");
    TestEntity* entity = nullptr;

    EXPECT_TRUE(entity = parent.addNestedEntity<TestEntity>(EntityTypeId::Api, "api"));
    ASSERT_EQ(parent.nested().size(), 1);
    ASSERT_NE(parent.nested().find("api"), parent.nested().end());
    EXPECT_EQ(parent.nested().find("api")->second, entity);
}

TEST(CommonEntityTest, Composite_Entity_Throws_Name_Conflict_Error_If_Added_Entity_Has_The_Same_Name_As_Existing)
{
    TestCompositeEntity entity(nullptr, EntityTypeId::Project, "project");
    EXPECT_TRUE(entity.addNestedEntity<TestEntity>(EntityTypeId::Api, "api"));
    EXPECT_NAME_CONFLICT_EXCEPTION(entity.addNestedEntity<TestDistinguishedEntity>(EntityTypeId::Services, "api"),
                                   EntityTypeId::Project,
                                   entity.dname(),
                                   "api");
}

TEST(CommonEntityTest, On_Nested_Entity_Added_Callback_Is_Invoked_For_Added_Entity)
{
    Entity* ptrInsideCb = nullptr;
    TestCompositeEntity parent(
        nullptr, EntityTypeId::Project, "project", [&ptrInsideCb](Entity* entity) { ptrInsideCb = entity; });

    Entity* entity = nullptr;

    EXPECT_TRUE(entity = parent.addNestedEntity<TestEntity>(EntityTypeId::Api, "api"));
    EXPECT_EQ(ptrInsideCb, entity);
}

TEST(CommonEntityTest, On_Nested_Entity_Added_Callbacks_Are_Merged_When_Set_For_Both_Parent_And_Child)
{
    int projectCallbackNum = 0;
    int namespaceCallbackNum = 0;
    auto projectCb = [&projectCallbackNum](Entity*) { ++projectCallbackNum; };
    auto namespaceCb = [&namespaceCallbackNum](Entity*) { ++namespaceCallbackNum; };

    TestCompositeEntity project(nullptr, EntityTypeId::Project, "project", projectCb);
    auto api = project.addNestedEntity<TestCompositeEntity>(EntityTypeId::Api, "api");

    EXPECT_EQ(projectCallbackNum, 1);
    EXPECT_EQ(namespaceCallbackNum, 0);

    auto ns1 = api->addNestedEntity<TestCompositeEntity>(EntityTypeId::Namespace, "namespace1", namespaceCb);

    EXPECT_EQ(projectCallbackNum, 2);
    EXPECT_EQ(namespaceCallbackNum, 0);

    auto ns2 = api->addNestedEntity<TestCompositeEntity>(EntityTypeId::Namespace, "namespace2");

    EXPECT_EQ(projectCallbackNum, 3);
    EXPECT_EQ(namespaceCallbackNum, 0);

    auto cls1 = ns1->addNestedEntity<TestCompositeEntity>(EntityTypeId::Class, "class1");

    EXPECT_EQ(projectCallbackNum, 4);
    EXPECT_EQ(namespaceCallbackNum, 1);

    cls1->addNestedEntity<TestCompositeEntity>(EntityTypeId::Method, "method1");

    EXPECT_EQ(projectCallbackNum, 5);
    EXPECT_EQ(namespaceCallbackNum, 2);

    ns2->addNestedEntity<TestCompositeEntity>(EntityTypeId::Class, "class2");

    EXPECT_EQ(projectCallbackNum, 6);
    EXPECT_EQ(namespaceCallbackNum, 2);
}

TEST(CommonEntityTest, General_Composite_Entity_Stores_Added_Structs)
{
    Project project;
    auto api = project.addApi();
    Struct* entity = nullptr;

    EXPECT_TRUE(entity = api->addStruct("Struct", "file.proto"));
    EXPECT_EQ(api->structs().size(), 1);
    ASSERT_NE(api->structs().find("Struct"), api->structs().end());
    EXPECT_EQ(api->structs().find("Struct")->second, entity);
}

TEST(CommonEntityTest, General_Composite_Entity_Stores_Added_Enums)
{
    Project project;
    auto api = project.addApi();
    Enum* entity = nullptr;

    EXPECT_TRUE(entity = api->addEnum("enum", "file.proto"));
    EXPECT_EQ(api->enums().size(), 1);
    ASSERT_NE(api->enums().find("enum"), api->enums().end());
    EXPECT_EQ(api->enums().find("enum")->second, entity);
}

TEST(CommonEntityTest, Struct_Type_Id_Is_Mapped_To_Predefined_Struct_Name_If_It_Exists)
{
    using enum StructTypeId;

    EXPECT_EQ(General, GetStructTypeId(GetPredefinedStructName(General)));
    EXPECT_EQ(Namespace_Desc, GetStructTypeId(GetPredefinedStructName(Namespace_Desc)));
    EXPECT_EQ(Class_Desc, GetStructTypeId(GetPredefinedStructName(Class_Desc)));
    EXPECT_EQ(Method_Desc, GetStructTypeId(GetPredefinedStructName(Method_Desc)));
    EXPECT_EQ(Service_Desc, GetStructTypeId(GetPredefinedStructName(Service_Desc)));
    EXPECT_EQ(Call_Message, GetStructTypeId(GetPredefinedStructName(Call_Message)));
    EXPECT_EQ(Result_Message, GetStructTypeId(GetPredefinedStructName(Result_Message)));
    EXPECT_EQ(Object_Id, GetStructTypeId(GetPredefinedStructName(Object_Id)));
    EXPECT_EQ(Static_Marker, GetStructTypeId(GetPredefinedStructName(Static_Marker)));
    EXPECT_EQ(Method_Params, GetStructTypeId(GetPredefinedStructName(Method_Params)));
    EXPECT_EQ(Method_Retval, GetStructTypeId(GetPredefinedStructName(Method_Retval)));
    EXPECT_EQ(Method_Exception, GetStructTypeId(GetPredefinedStructName(Method_Exception)));
    EXPECT_EQ(Service_Config, GetStructTypeId(GetPredefinedStructName(Service_Config)));
    EXPECT_EQ(Service_Implements, GetStructTypeId(GetPredefinedStructName(Service_Implements)));
    EXPECT_EQ(Service_Invokes, GetStructTypeId(GetPredefinedStructName(Service_Invokes)));
    EXPECT_FALSE(GetPredefinedStructName(StructTypeId::General));
}

TEST(CommonEntityTest, GetPredefinedStructName_Returns_Nullptr_For_General_Struct_Type)
{
    EXPECT_FALSE(GetPredefinedStructName(StructTypeId::General));
}

TEST(CommonEntityTest, GetPredefinedStructName_Returns_Nullptr_For_Unknown_Struct_Type)
{
    EXPECT_FALSE(GetPredefinedStructName(static_cast<StructTypeId>(0)));
}

TEST(CommonEntityTest, GetStructTypeId_Returns_Predefined_Type_For_Properly_Named_And_Nested_Struct_Type)
{
    using enum StructTypeId;

    EXPECT_EQ(Namespace_Desc, GetStructTypeId(GetPredefinedStructName(Namespace_Desc), EntityTypeId::Namespace));
    EXPECT_EQ(Class_Desc, GetStructTypeId(GetPredefinedStructName(Class_Desc), EntityTypeId::Class));
    EXPECT_EQ(Method_Desc, GetStructTypeId(GetPredefinedStructName(Method_Desc), EntityTypeId::Method));
    EXPECT_EQ(Service_Desc, GetStructTypeId(GetPredefinedStructName(Service_Desc), EntityTypeId::Service));
    EXPECT_EQ(Call_Message, GetStructTypeId(GetPredefinedStructName(Call_Message), EntityTypeId::Api));
    EXPECT_EQ(Result_Message, GetStructTypeId(GetPredefinedStructName(Result_Message), EntityTypeId::Api));
    EXPECT_EQ(Object_Id,
              GetStructTypeId(GetPredefinedStructName(Object_Id), EntityTypeId::Struct, StructTypeId::Class_Desc));
    EXPECT_EQ(Static_Marker,
              GetStructTypeId(GetPredefinedStructName(Static_Marker), EntityTypeId::Struct, StructTypeId::Method_Desc));
    EXPECT_EQ(Method_Params,
              GetStructTypeId(GetPredefinedStructName(Method_Params), EntityTypeId::Struct, StructTypeId::Method_Desc));
    EXPECT_EQ(Method_Retval,
              GetStructTypeId(GetPredefinedStructName(Method_Retval), EntityTypeId::Struct, StructTypeId::Method_Desc));
    EXPECT_EQ(Method_Exception, GetStructTypeId(GetPredefinedStructName(Method_Exception), EntityTypeId::Api));
    EXPECT_EQ(
        Service_Config,
        GetStructTypeId(GetPredefinedStructName(Service_Config), EntityTypeId::Struct, StructTypeId::Service_Desc));
    EXPECT_EQ(
        Service_Implements,
        GetStructTypeId(GetPredefinedStructName(Service_Implements), EntityTypeId::Struct, StructTypeId::Service_Desc));
    EXPECT_EQ(
        Service_Invokes,
        GetStructTypeId(GetPredefinedStructName(Service_Invokes), EntityTypeId::Struct, StructTypeId::Service_Desc));
}

TEST(CommonEntityTest, GetStructTypeId_Returns_General_Type_For_Incorrectly_Nested_Struct_Type_With_Predefined_Name)
{
    using enum StructTypeId;

    EXPECT_EQ(General, GetStructTypeId(GetPredefinedStructName(Namespace_Desc), EntityTypeId::Project));
    EXPECT_EQ(General, GetStructTypeId(GetPredefinedStructName(Class_Desc), EntityTypeId::Project));
    EXPECT_EQ(General, GetStructTypeId(GetPredefinedStructName(Method_Desc), EntityTypeId::Project));
    EXPECT_EQ(General, GetStructTypeId(GetPredefinedStructName(Service_Desc), EntityTypeId::Project));
    EXPECT_EQ(General, GetStructTypeId(GetPredefinedStructName(Call_Message), EntityTypeId::Project));
    EXPECT_EQ(General, GetStructTypeId(GetPredefinedStructName(Result_Message), EntityTypeId::Project));
    EXPECT_EQ(General, GetStructTypeId(GetPredefinedStructName(Object_Id), EntityTypeId::Project));
    EXPECT_EQ(General, GetStructTypeId(GetPredefinedStructName(Static_Marker), EntityTypeId::Project));
    EXPECT_EQ(General, GetStructTypeId(GetPredefinedStructName(Method_Params), EntityTypeId::Project));
    EXPECT_EQ(General, GetStructTypeId(GetPredefinedStructName(Method_Retval), EntityTypeId::Project));
    EXPECT_EQ(General, GetStructTypeId(GetPredefinedStructName(Method_Exception), EntityTypeId::Project));
    EXPECT_EQ(General, GetStructTypeId(GetPredefinedStructName(Service_Config), EntityTypeId::Project));
    EXPECT_EQ(General, GetStructTypeId(GetPredefinedStructName(Service_Implements), EntityTypeId::Project));
    EXPECT_EQ(General, GetStructTypeId(GetPredefinedStructName(Service_Invokes), EntityTypeId::Project));
}

TEST(CommonEntityTest, GetStructTypeId_Returns_General_Type_If_Parent_Struct_Type_Is_Required_But_Not_Specified)
{
    using enum StructTypeId;

    EXPECT_EQ(General, GetStructTypeId(GetPredefinedStructName(Object_Id), EntityTypeId::Struct));
    EXPECT_EQ(General, GetStructTypeId(GetPredefinedStructName(Static_Marker), EntityTypeId::Struct));
    EXPECT_EQ(General, GetStructTypeId(GetPredefinedStructName(Method_Params), EntityTypeId::Struct));
    EXPECT_EQ(General, GetStructTypeId(GetPredefinedStructName(Method_Retval), EntityTypeId::Struct));
    EXPECT_EQ(General, GetStructTypeId(GetPredefinedStructName(Service_Config), EntityTypeId::Struct));
    EXPECT_EQ(General, GetStructTypeId(GetPredefinedStructName(Service_Implements), EntityTypeId::Struct));
    EXPECT_EQ(General, GetStructTypeId(GetPredefinedStructName(Service_Invokes), EntityTypeId::Struct));
}

TEST(CommonEntityTest, GetStructTypeId_Returns_General_Type_If_Parent_Struct_Type_Is_Not_The_Exepected_One)
{
    using enum StructTypeId;

    EXPECT_EQ(General,
              GetStructTypeId(GetPredefinedStructName(Object_Id), EntityTypeId::Struct, StructTypeId::General));
    EXPECT_EQ(
        General,
        GetStructTypeId(GetPredefinedStructName(Method_Params), EntityTypeId::Struct, StructTypeId::Namespace_Desc));
    EXPECT_EQ(
        General,
        GetStructTypeId(GetPredefinedStructName(Method_Retval), EntityTypeId::Struct, StructTypeId::Service_Desc));
    EXPECT_EQ(General,
              GetStructTypeId(GetPredefinedStructName(Static_Marker), EntityTypeId::Struct, StructTypeId::Class_Desc));
    EXPECT_EQ(
        General,
        GetStructTypeId(GetPredefinedStructName(Service_Config), EntityTypeId::Struct, StructTypeId::Method_Desc));
    EXPECT_EQ(
        General,
        GetStructTypeId(GetPredefinedStructName(Service_Implements), EntityTypeId::Struct, StructTypeId::Method_Desc));
    EXPECT_EQ(
        General,
        GetStructTypeId(GetPredefinedStructName(Service_Invokes), EntityTypeId::Struct, StructTypeId::Method_Desc));
}

TEST(CommonEntityTest, GetStructTypeId_Returns_General_Type_If_Name_Is_Not_Predefined)
{
    using enum StructTypeId;

    EXPECT_EQ(General, GetStructTypeId("GeneralStruct", EntityTypeId::Api));
}

TEST(CommonEntityTest, GetStructTypeId_Returns_General_Type_If_Name_Is_Not_Set)
{
    using enum StructTypeId;

    EXPECT_EQ(General, GetStructTypeId(nullptr, EntityTypeId::Api));
    EXPECT_EQ(General, GetStructTypeId(std::string_view{}, EntityTypeId::Api));
}

TEST(CommonEntityTest, GetFieldTypeIdStr_Returns_Non_Nullptr_For_All_Known_Field_Types)
{
    using enum FieldTypeId;

    EXPECT_TRUE(GetFieldTypeIdStr(Bool));
    EXPECT_TRUE(GetFieldTypeIdStr(Int32));
    EXPECT_TRUE(GetFieldTypeIdStr(Sint32));
    EXPECT_TRUE(GetFieldTypeIdStr(Sfixed32));
    EXPECT_TRUE(GetFieldTypeIdStr(Uint32));
    EXPECT_TRUE(GetFieldTypeIdStr(Fixed32));
    EXPECT_TRUE(GetFieldTypeIdStr(Int64));
    EXPECT_TRUE(GetFieldTypeIdStr(Sint64));
    EXPECT_TRUE(GetFieldTypeIdStr(Sfixed64));
    EXPECT_TRUE(GetFieldTypeIdStr(Uint64));
    EXPECT_TRUE(GetFieldTypeIdStr(Fixed64));
    EXPECT_TRUE(GetFieldTypeIdStr(Float));
    EXPECT_TRUE(GetFieldTypeIdStr(Double));
    EXPECT_TRUE(GetFieldTypeIdStr(String));
    EXPECT_TRUE(GetFieldTypeIdStr(Bytes));
    EXPECT_TRUE(GetFieldTypeIdStr(Map));
    EXPECT_TRUE(GetFieldTypeIdStr(Enum));
    EXPECT_TRUE(GetFieldTypeIdStr(Message));
}

TEST(CommonEntityTest, GetFieldTypeIdStr_Returns_Nullptr_For_Unknown_Field_Type)
{
    EXPECT_FALSE(GetFieldTypeIdStr(static_cast<FieldTypeId>(0)));
}

TEST(CommonEntityTest, IsScalarFieldType_Returns_True_For_Scalar_Types)
{
    using enum FieldTypeId;

    EXPECT_TRUE(IsScalarFieldType(Bool));
    EXPECT_TRUE(IsScalarFieldType(Int32));
    EXPECT_TRUE(IsScalarFieldType(Sint32));
    EXPECT_TRUE(IsScalarFieldType(Sfixed32));
    EXPECT_TRUE(IsScalarFieldType(Uint32));
    EXPECT_TRUE(IsScalarFieldType(Fixed32));
    EXPECT_TRUE(IsScalarFieldType(Int64));
    EXPECT_TRUE(IsScalarFieldType(Sint64));
    EXPECT_TRUE(IsScalarFieldType(Sfixed64));
    EXPECT_TRUE(IsScalarFieldType(Uint64));
    EXPECT_TRUE(IsScalarFieldType(Fixed64));
    EXPECT_TRUE(IsScalarFieldType(Float));
    EXPECT_TRUE(IsScalarFieldType(Double));
    EXPECT_TRUE(IsScalarFieldType(String));
    EXPECT_TRUE(IsScalarFieldType(Bytes));
}

TEST(CommonEntityTest, IsScalarFieldType_Returns_False_For_Non_Scalar_Types)
{
    using enum FieldTypeId;

    EXPECT_FALSE(IsScalarFieldType(Map));
    EXPECT_FALSE(IsScalarFieldType(Enum));
    EXPECT_FALSE(IsScalarFieldType(Message));
}

TEST(CommonEntityTest, IsScalarFieldType_Returns_False_For_Unknown_Type)
{
    EXPECT_FALSE(IsScalarFieldType(static_cast<FieldTypeId>(0)));
}
}} // namespace busrpc::test