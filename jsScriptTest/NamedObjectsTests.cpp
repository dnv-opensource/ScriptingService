#include <gtest/gtest.h>
#include "ScriptingServiceFactory.h"
#include "Services/ScopedServiceProvider.h"
#include "Reflection/Attributes/NameAttribute.h"
#include "Reflection/Classes/Class.h"
#include "jsScript/jsStack.h"

#include "Scripting/INameService.h"
using namespace DNVS::MoFa::Reflection;
using namespace DNVS::MoFa::Services;
using namespace DNVS::MoFa::Scripting;

class NamedObjectsTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_scriptingService = ScriptingServiceFactory().CreateScriptingService();
        m_typeLibrary = m_scriptingService->GetTypeLibrary();
        m_sp.RegisterService(m_typeLibrary);
    }
    ~NamedObjectsTests() {
        Reset();
    }
    void Reset()
    {
        m_sp.ClearAllServices();
        m_scriptingService.reset();
        m_typeLibrary.reset();
    }
    ScopedServiceProvider m_sp;
    ScriptingServicePointer m_scriptingService;
    TypeLibraries::TypeLibraryPointer m_typeLibrary;
};

class ObjectWithName {
public:
    ObjectWithName() 
    {
        counter++; 
        auto service = jsStack::stack()->GetTypeLibrary()->GetServiceProvider().TryGetService<INewObjectScope>(); 
        if(service)
            service->AddNewObject(Objects::Object(jsStack::stack()->GetTypeLibrary(), this)); 
    }
    ~ObjectWithName() { counter--; }
    std::string GetName() const { return m_name; }
    bool SetName(const std::string& name) { m_name = name; return true; }
    static int counter;
private:
    std::string m_name;
};

int ObjectWithName::counter = 0;
void DoReflect(TypeLibraries::TypeLibraryPointer typeLibrary, ObjectWithName**)
{
    using namespace Classes;
    Class<ObjectWithName, std::shared_ptr<ObjectWithName>> cls(typeLibrary, "ObjectWithName");
    cls.Constructor();
    cls.AddAttribute<NameAttribute>(
        [](const ObjectWithName* object) {return object->GetName(); },
        [](ObjectWithName* object, const std::string& name) { return object->SetName(name); });
}
TEST_F(NamedObjectsTests, CreateNamedObjectByRegularAssignment)
{
    Reflect<ObjectWithName>(m_typeLibrary);
    EXPECT_EQ("a", m_scriptingService->Execute("a = ObjectWithName();").As<std::shared_ptr<ObjectWithName>>()->GetName());
}

TEST_F(NamedObjectsTests, CheckHasMember)
{
    Reflect<ObjectWithName>(m_typeLibrary);
    EXPECT_FALSE(m_scriptingService->HasMember("a"));
    m_scriptingService->Execute("a = ObjectWithName();");
    EXPECT_TRUE(m_scriptingService->HasMember("a"));
}

TEST_F(NamedObjectsTests, CreateArrayThenAssignObjectToArrayThenAssignToNamedVariable_NoRename)
{
    Reflect<ObjectWithName>(m_typeLibrary);
    m_scriptingService->Execute("a = Array();");
    m_scriptingService->Execute("a[0] = ObjectWithName();");
    EXPECT_EQ("", m_scriptingService->Execute("b = a[0];").As<std::shared_ptr<ObjectWithName>>()->GetName());
}

TEST_F(NamedObjectsTests, CreateArrayWithObjectThenAssignToNamedVariable_NoRename)
{
    Reflect<ObjectWithName>(m_typeLibrary);
    m_scriptingService->Execute("a = Array(ObjectWithName());");
    EXPECT_EQ("", m_scriptingService->Execute("b = a[0];").As<std::shared_ptr<ObjectWithName>>()->GetName());
}

TEST_F(NamedObjectsTests, AddNamedObjectToScripting)
{
    Reflect<ObjectWithName>(m_typeLibrary);
    std::shared_ptr<ObjectWithName> object(new ObjectWithName);
    object->SetName("a");
    m_scriptingService->RenameMember(Objects::Object(m_typeLibrary, object), "a");
    EXPECT_TRUE(m_scriptingService->HasMember("a"));
    EXPECT_TRUE(m_scriptingService->Execute("a;").IsConvertibleTo<std::shared_ptr<ObjectWithName>>());
}

TEST_F(NamedObjectsTests, DeleteObject)
{
    auto typeLibrary = m_typeLibrary;
    Reflect<ObjectWithName>(m_typeLibrary);
    m_scriptingService->Execute("a = ObjectWithName();");
    EXPECT_EQ(1, ObjectWithName::counter);
    m_scriptingService->DeleteMember("a");
    EXPECT_EQ(0, ObjectWithName::counter);
    Reset();
    EXPECT_TRUE(typeLibrary.unique());
}

TEST_F(NamedObjectsTests, DoNothing_CheckDelete)
{
    auto typeLibrary = m_typeLibrary;
    Reset();
    EXPECT_TRUE(typeLibrary.unique());
}

TEST_F(NamedObjectsTests, WrapPointerGetSmartPointer)
{
    Reflect<ObjectWithName>(m_typeLibrary);
    ObjectWithName* object = new ObjectWithName;
    object->SetName("a");
    m_scriptingService->RenameMember(Objects::Object(m_typeLibrary, object), "a");
    EXPECT_NO_THROW(m_scriptingService->Execute("a;").As<std::shared_ptr<ObjectWithName>>());
}