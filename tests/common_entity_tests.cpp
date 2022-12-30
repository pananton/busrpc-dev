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

TEST(CommonEntityTest, GetEntityTypeIdStr_Returns_Non_Nullptr_For_Known_Entity_Type)
{
    EXPECT_TRUE(GetEntityTypeIdStr(EntityTypeId::Project));
    EXPECT_TRUE(GetEntityTypeIdStr(EntityTypeId::Api));
    EXPECT_TRUE(GetEntityTypeIdStr(EntityTypeId::Implementation));
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

TEST(CommonEntityTest, GetEntityTypeIdStr_Returns_Nullptr_For_Unknown_Entity_Type)
{
    EXPECT_FALSE(GetEntityTypeIdStr(static_cast<EntityTypeId>(0)));
}

TEST(CommonEntityTest, IsValidEntityName_Returns_True_For_Name_Consisting_Of_Alphanumericals_And_Underscores)
{
    EXPECT_TRUE(IsValidEntityName("a"));
    EXPECT_TRUE(IsValidEntityName("a1"));
    EXPECT_TRUE(IsValidEntityName("_a"));
    EXPECT_TRUE(IsValidEntityName("a_"));
    EXPECT_TRUE(IsValidEntityName("_1"));
    EXPECT_TRUE(IsValidEntityName("_"));
}

TEST(CommonEntityTest, IsValidEntityName_Returns_False_For_Name_Which_Contains_Prohibited_Characters)
{
    EXPECT_FALSE(IsValidEntityName(".a"));
    EXPECT_FALSE(IsValidEntityName("a.b"));
    EXPECT_FALSE(IsValidEntityName("a."));
}

TEST(CommonEntityTest, IsValidEntityName_Returns_False_For_Name_Which_Stars_With_Digit)
{
    EXPECT_FALSE(IsValidEntityName("0a"));
}

TEST(CommonEntityTest, IsValidEntityName_Returns_False_For_Empty_Name)
{
    EXPECT_FALSE(IsValidEntityName(""));
}

TEST(CommonEntityTest, Entity_Ctor_Correctly_Initializes_Object)
{
    std::string entityName = "entity";
    TestEntity entity(nullptr, EntityTypeId::Struct, entityName);

    EXPECT_EQ(entity.type(), EntityTypeId::Struct);
    EXPECT_EQ(entity.name(), entityName);
    EXPECT_EQ(entity.dname(), entity.name());
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

        EXPECT_EQ(entity.dname(), JoinStrings(parent.dname(), ".", entity.name()));
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

            EXPECT_EQ(entity.dname(), JoinStrings(parent.dname(), ".", entity.name()));
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

TEST(CommonEntityTest, Entity_Ctor_Throws_If_Entity_Name_Is_Invalid)
{
    EXPECT_ENTITY_EXCEPTION(TestEntity(nullptr, EntityTypeId::Struct, ""), EntityTypeId::Struct, "");
}

TEST(ComponentEntityTest, OrderEntitiesByNameAsc_Returns_Correct_Result)
{
    TestEntity e1(nullptr, EntityTypeId::Class, "class1");
    TestEntity e2(nullptr, EntityTypeId::Class, "class2");

    EXPECT_TRUE(OrderEntitiesByNameAsc()(&e1, &e2));
    EXPECT_FALSE(OrderEntitiesByNameAsc()(&e2, &e1));
    EXPECT_TRUE(OrderEntitiesByNameAsc()(&e1, "class2"));
    EXPECT_FALSE(OrderEntitiesByNameAsc()(&e2, "class1"));
    EXPECT_TRUE(OrderEntitiesByNameAsc()("class1", &e2));
    EXPECT_FALSE(OrderEntitiesByNameAsc()("class2", &e1));
}

TEST(CommonEntityTest, Composite_Entity_Stores_Added_Nested_Entities)
{
    TestCompositeEntity parent(nullptr, EntityTypeId::Project, "project");
    TestEntity* entity = nullptr;

    EXPECT_TRUE(entity = parent.addNestedEntity<TestEntity>(EntityTypeId::Api, "api"));
    ASSERT_EQ(parent.nested().size(), 1);
    ASSERT_NE(parent.nested().find("api"), parent.nested().end());
    EXPECT_EQ(*(parent.nested().find("api")), entity);
}

TEST(CommonEntityTest, Composite_Entity_Throws_Name_Conflict_Error_If_Added_Entity_Has_The_Same_Name_As_Existing)
{
    TestCompositeEntity entity(nullptr, EntityTypeId::Project, "project");
    EXPECT_TRUE(entity.addNestedEntity<TestEntity>(EntityTypeId::Api, "api"));
    EXPECT_NAME_CONFLICT_EXCEPTION(entity.addNestedEntity<TestEntity>(EntityTypeId::Implementation, "api"),
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
    EXPECT_EQ(*(api->structs().find("Struct")), entity);
}

TEST(CommonEntityTest, General_Composite_Entity_Stores_Added_Enums)
{
    Project project;
    auto api = project.addApi();
    Enum* entity = nullptr;

    EXPECT_TRUE(entity = api->addEnum("enum", "file.proto"));
    EXPECT_EQ(api->enums().size(), 1);
    ASSERT_NE(api->enums().find("enum"), api->enums().end());
    EXPECT_EQ(*(api->enums().find("enum")), entity);
}

TEST(CommonEntityTest, Struct_Type_Id_Is_Mapped_To_Predefined_Struct_Name_If_It_Exists)
{
    EXPECT_EQ(StructTypeId::General, GetStructTypeId(GetPredefinedStructName(StructTypeId::General)));
    EXPECT_EQ(StructTypeId::Exception, GetStructTypeId(GetPredefinedStructName(StructTypeId::Exception)));
    EXPECT_EQ(StructTypeId::Call_Message, GetStructTypeId(GetPredefinedStructName(StructTypeId::Call_Message)));
    EXPECT_EQ(StructTypeId::Result_Message, GetStructTypeId(GetPredefinedStructName(StructTypeId::Result_Message)));
    EXPECT_EQ(StructTypeId::Namespace_Desc, GetStructTypeId(GetPredefinedStructName(StructTypeId::Namespace_Desc)));
    EXPECT_EQ(StructTypeId::Class_Desc, GetStructTypeId(GetPredefinedStructName(StructTypeId::Class_Desc)));
    EXPECT_EQ(StructTypeId::Class_Object_Id, GetStructTypeId(GetPredefinedStructName(StructTypeId::Class_Object_Id)));
    EXPECT_EQ(StructTypeId::Method_Desc, GetStructTypeId(GetPredefinedStructName(StructTypeId::Method_Desc)));
    EXPECT_EQ(StructTypeId::Method_Params, GetStructTypeId(GetPredefinedStructName(StructTypeId::Method_Params)));
    EXPECT_EQ(StructTypeId::Method_Retval, GetStructTypeId(GetPredefinedStructName(StructTypeId::Method_Retval)));
    EXPECT_EQ(StructTypeId::Method_Static_Marker,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Method_Static_Marker)));
    EXPECT_EQ(StructTypeId::Service_Desc, GetStructTypeId(GetPredefinedStructName(StructTypeId::Service_Desc)));
    EXPECT_EQ(StructTypeId::Service_Config, GetStructTypeId(GetPredefinedStructName(StructTypeId::Service_Config)));
    EXPECT_EQ(StructTypeId::Service_Implements,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Service_Implements)));
    EXPECT_EQ(StructTypeId::Service_Invokes, GetStructTypeId(GetPredefinedStructName(StructTypeId::Service_Invokes)));
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
    EXPECT_EQ(StructTypeId::Exception,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Exception), EntityTypeId::Project));
    EXPECT_EQ(StructTypeId::Call_Message,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Call_Message), EntityTypeId::Project));
    EXPECT_EQ(StructTypeId::Result_Message,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Result_Message), EntityTypeId::Project));
    EXPECT_EQ(StructTypeId::Namespace_Desc,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Namespace_Desc), EntityTypeId::Namespace));
    EXPECT_EQ(StructTypeId::Class_Desc,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Class_Desc), EntityTypeId::Class));
    EXPECT_EQ(StructTypeId::Class_Object_Id,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Class_Object_Id),
                              EntityTypeId::Struct,
                              StructTypeId::Class_Desc));
    EXPECT_EQ(StructTypeId::Method_Desc,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Method_Desc), EntityTypeId::Method));
    EXPECT_EQ(StructTypeId::Method_Params,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Method_Params),
                              EntityTypeId::Struct,
                              StructTypeId::Method_Desc));
    EXPECT_EQ(StructTypeId::Method_Retval,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Method_Retval),
                              EntityTypeId::Struct,
                              StructTypeId::Method_Desc));
    EXPECT_EQ(StructTypeId::Method_Static_Marker,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Method_Static_Marker),
                              EntityTypeId::Struct,
                              StructTypeId::Method_Desc));
    EXPECT_EQ(StructTypeId::Service_Desc,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Service_Desc), EntityTypeId::Service));
    EXPECT_EQ(StructTypeId::Service_Config,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Service_Config),
                              EntityTypeId::Struct,
                              StructTypeId::Service_Desc));
    EXPECT_EQ(StructTypeId::Service_Implements,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Service_Implements),
                              EntityTypeId::Struct,
                              StructTypeId::Service_Desc));
    EXPECT_EQ(StructTypeId::Service_Invokes,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Service_Invokes),
                              EntityTypeId::Struct,
                              StructTypeId::Service_Desc));
}

TEST(CommonEntityTest, GetStructTypeId_Returns_General_Type_For_Incorrectly_Nested_Struct_Type_With_Predefined_Name)
{
    EXPECT_EQ(StructTypeId::General,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Exception), EntityTypeId::Implementation));
    EXPECT_EQ(StructTypeId::General,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Call_Message), EntityTypeId::Implementation));
    EXPECT_EQ(StructTypeId::General,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Result_Message), EntityTypeId::Implementation));
    EXPECT_EQ(StructTypeId::General,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Namespace_Desc), EntityTypeId::Implementation));
    EXPECT_EQ(StructTypeId::General,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Class_Desc), EntityTypeId::Implementation));
    EXPECT_EQ(StructTypeId::General,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Class_Object_Id), EntityTypeId::Implementation));
    EXPECT_EQ(StructTypeId::General,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Method_Desc), EntityTypeId::Implementation));
    EXPECT_EQ(StructTypeId::General,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Method_Params), EntityTypeId::Implementation));
    EXPECT_EQ(StructTypeId::General,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Method_Retval), EntityTypeId::Implementation));
    EXPECT_EQ(
        StructTypeId::General,
        GetStructTypeId(GetPredefinedStructName(StructTypeId::Method_Static_Marker), EntityTypeId::Implementation));
    EXPECT_EQ(StructTypeId::General,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Service_Desc), EntityTypeId::Implementation));
    EXPECT_EQ(StructTypeId::General,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Service_Config), EntityTypeId::Implementation));
    EXPECT_EQ(StructTypeId::General,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Service_Implements), EntityTypeId::Implementation));
    EXPECT_EQ(StructTypeId::General,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Service_Invokes), EntityTypeId::Implementation));
}

TEST(CommonEntityTest, GetStructTypeId_Returns_General_Type_If_Parent_Struct_Type_Is_Required_But_Not_Specified)
{
    EXPECT_EQ(StructTypeId::General,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Class_Object_Id), EntityTypeId::Struct));
    EXPECT_EQ(StructTypeId::General,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Method_Params), EntityTypeId::Struct));
    EXPECT_EQ(StructTypeId::General,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Method_Retval), EntityTypeId::Struct));
    EXPECT_EQ(StructTypeId::General,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Method_Static_Marker), EntityTypeId::Struct));
    EXPECT_EQ(StructTypeId::General,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Service_Config), EntityTypeId::Struct));
    EXPECT_EQ(StructTypeId::General,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Service_Implements), EntityTypeId::Struct));
    EXPECT_EQ(StructTypeId::General,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Service_Invokes), EntityTypeId::Struct));
}

TEST(CommonEntityTest, GetStructTypeId_Returns_General_Type_If_Parent_Struct_Type_Is_Not_The_Exepected_One)
{
    EXPECT_EQ(StructTypeId::General,
              GetStructTypeId(
                  GetPredefinedStructName(StructTypeId::Class_Object_Id), EntityTypeId::Struct, StructTypeId::General));
    EXPECT_EQ(StructTypeId::General,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Method_Params),
                              EntityTypeId::Struct,
                              StructTypeId::Namespace_Desc));
    EXPECT_EQ(StructTypeId::General,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Method_Retval),
                              EntityTypeId::Struct,
                              StructTypeId::Service_Desc));
    EXPECT_EQ(StructTypeId::General,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Method_Static_Marker),
                              EntityTypeId::Struct,
                              StructTypeId::Class_Desc));
    EXPECT_EQ(StructTypeId::General,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Service_Config),
                              EntityTypeId::Struct,
                              StructTypeId::Method_Desc));
    EXPECT_EQ(StructTypeId::General,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Service_Implements),
                              EntityTypeId::Struct,
                              StructTypeId::Method_Desc));
    EXPECT_EQ(StructTypeId::General,
              GetStructTypeId(GetPredefinedStructName(StructTypeId::Service_Invokes),
                              EntityTypeId::Struct,
                              StructTypeId::Method_Desc));
}

TEST(CommonEntityTest, GetStructTypeId_Returns_General_Type_If_Name_Is_Not_Predefined)
{
    EXPECT_EQ(StructTypeId::General, GetStructTypeId("GeneralStruct", EntityTypeId::Api));
}

TEST(CommonEntityTest, GetStructTypeId_Returns_General_Type_If_Name_Is_Not_Set)
{
    EXPECT_EQ(StructTypeId::General, GetStructTypeId(nullptr, EntityTypeId::Api));
    EXPECT_EQ(StructTypeId::General, GetStructTypeId(std::string_view{}, EntityTypeId::Api));
}

TEST(CommonEntityTest, GetFieldTypeIdStr_Returns_Non_Nullptr_For_All_Known_Field_Types)
{
    EXPECT_TRUE(GetFieldTypeIdStr(FieldTypeId::Bool));
    EXPECT_TRUE(GetFieldTypeIdStr(FieldTypeId::Int32));
    EXPECT_TRUE(GetFieldTypeIdStr(FieldTypeId::Sint32));
    EXPECT_TRUE(GetFieldTypeIdStr(FieldTypeId::Sfixed32));
    EXPECT_TRUE(GetFieldTypeIdStr(FieldTypeId::Uint32));
    EXPECT_TRUE(GetFieldTypeIdStr(FieldTypeId::Fixed32));
    EXPECT_TRUE(GetFieldTypeIdStr(FieldTypeId::Int64));
    EXPECT_TRUE(GetFieldTypeIdStr(FieldTypeId::Sint64));
    EXPECT_TRUE(GetFieldTypeIdStr(FieldTypeId::Sfixed64));
    EXPECT_TRUE(GetFieldTypeIdStr(FieldTypeId::Uint64));
    EXPECT_TRUE(GetFieldTypeIdStr(FieldTypeId::Fixed64));
    EXPECT_TRUE(GetFieldTypeIdStr(FieldTypeId::Float));
    EXPECT_TRUE(GetFieldTypeIdStr(FieldTypeId::Double));
    EXPECT_TRUE(GetFieldTypeIdStr(FieldTypeId::String));
    EXPECT_TRUE(GetFieldTypeIdStr(FieldTypeId::Bytes));
    EXPECT_TRUE(GetFieldTypeIdStr(FieldTypeId::Map));
    EXPECT_TRUE(GetFieldTypeIdStr(FieldTypeId::Enum));
    EXPECT_TRUE(GetFieldTypeIdStr(FieldTypeId::Message));
}

TEST(CommonEntityTest, GetFieldTypeIdStr_Returns_Nullptr_For_Unknown_Field_Type)
{
    EXPECT_FALSE(GetFieldTypeIdStr(static_cast<FieldTypeId>(0)));
}

TEST(CommonEntityTest, IsScalarFieldType_Returns_True_For_Scalar_Types)
{
    EXPECT_TRUE(IsScalarFieldType(FieldTypeId::Bool));
    EXPECT_TRUE(IsScalarFieldType(FieldTypeId::Int32));
    EXPECT_TRUE(IsScalarFieldType(FieldTypeId::Sint32));
    EXPECT_TRUE(IsScalarFieldType(FieldTypeId::Sfixed32));
    EXPECT_TRUE(IsScalarFieldType(FieldTypeId::Uint32));
    EXPECT_TRUE(IsScalarFieldType(FieldTypeId::Fixed32));
    EXPECT_TRUE(IsScalarFieldType(FieldTypeId::Int64));
    EXPECT_TRUE(IsScalarFieldType(FieldTypeId::Sint64));
    EXPECT_TRUE(IsScalarFieldType(FieldTypeId::Sfixed64));
    EXPECT_TRUE(IsScalarFieldType(FieldTypeId::Uint64));
    EXPECT_TRUE(IsScalarFieldType(FieldTypeId::Fixed64));
    EXPECT_TRUE(IsScalarFieldType(FieldTypeId::Float));
    EXPECT_TRUE(IsScalarFieldType(FieldTypeId::Double));
    EXPECT_TRUE(IsScalarFieldType(FieldTypeId::String));
    EXPECT_TRUE(IsScalarFieldType(FieldTypeId::Bytes));
}

TEST(CommonEntityTest, IsScalarFieldType_Returns_False_For_Non_Scalar_Types)
{
    EXPECT_FALSE(IsScalarFieldType(FieldTypeId::Map));
    EXPECT_FALSE(IsScalarFieldType(FieldTypeId::Enum));
    EXPECT_FALSE(IsScalarFieldType(FieldTypeId::Message));
}

TEST(CommonEntityTest, IsScalarFieldType_Returns_False_For_Unknown_Type)
{
    EXPECT_FALSE(IsScalarFieldType(static_cast<FieldTypeId>(0)));
}

TEST(CommonEntityTest, IsEncodableField_Returns_True_For_Non_Floating_Scalars_And_Enum)
{
    EXPECT_TRUE(IsEncodableField(FieldTypeId::Int32));
    EXPECT_TRUE(IsEncodableField(FieldTypeId::String));
    EXPECT_TRUE(IsEncodableField(FieldTypeId::Bytes));
    EXPECT_TRUE(IsEncodableField(FieldTypeId::Enum));

    EXPECT_TRUE(
        IsEncodableField(FieldTypeId::Int32, FieldFlags::Optional | FieldFlags::Observable | FieldFlags::Hashed));
    EXPECT_TRUE(
        IsEncodableField(FieldTypeId::String, FieldFlags::Optional | FieldFlags::Observable | FieldFlags::Hashed));
    EXPECT_TRUE(
        IsEncodableField(FieldTypeId::Bytes, FieldFlags::Optional | FieldFlags::Observable | FieldFlags::Hashed));
    EXPECT_TRUE(
        IsEncodableField(FieldTypeId::Enum, FieldFlags::Optional | FieldFlags::Observable | FieldFlags::Hashed));
}

TEST(CommonEntityTest, IsEncodableField_Returns_False_For_Non_Scalar_Types_Except_Enum)
{
    EXPECT_FALSE(IsEncodableField(FieldTypeId::Map));
    EXPECT_FALSE(IsEncodableField(FieldTypeId::Message));
}

TEST(CommonEntityTest, IsEncodableField_Returns_False_For_Floating_Point_Types)
{
    EXPECT_FALSE(IsEncodableField(FieldTypeId::Float));
    EXPECT_FALSE(IsEncodableField(FieldTypeId::Double));
}

TEST(CommonEntityTest, IsEncodableField_Returns_False_For_Repeated_Types)
{
    EXPECT_FALSE(IsEncodableField(FieldTypeId::Int32, FieldFlags::Repeated));
}

TEST(CommonEntityTest, IsEncodableField_Returns_False_For_Types_Which_Are_Part_Of_Oneof)
{
    EXPECT_FALSE(IsEncodableField(FieldTypeId::Int32, FieldFlags::None, "oneofName"));
}
}} // namespace busrpc::test
