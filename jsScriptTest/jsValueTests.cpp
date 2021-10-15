#include <Services/ScopedServiceProvider.h>
#include "GoogleTest\CppunitAdapter.h"
#include "Reflection\TypeLibraries\TypeLibraryFactory.h"
#include "jsScript/jsScriptable.h"
#include "jsScript/jsClass.h"
#include "Reflection/Objects/Object.h"
#include "ScriptingServiceFactory.h"
using namespace DNVS::MoFa::Scripting;

TEST(jsValueTests,TestArrayAsFunction)
{
    DNVS::MoFa::Services::ScopedServiceProvider sp;
    auto scriptingService = ScriptingServiceFactory().CreateScriptingService();
    sp.RegisterService<IScriptingService>(scriptingService);
    EXPECT_NO_THROW(scriptingService->Execute("AA=Array(3,4,12);"));
    EXPECT_NO_THROW(scriptingService->Execute("i=2;"));
    EXPECT_THROW(scriptingService->Execute("a=AA(i);"), std::exception);
}

class MyValue : public jsScriptable
{
public:
    MyValue(bool& alive) : m_alive(&alive) { *m_alive = true; }
    MyValue(const MyValue& other) : jsScriptable(other) , m_alive(nullptr) {}
    MyValue(const jsAutomation& automation) : jsScriptable(automation) , m_alive(nullptr) {}
    ~MyValue()
    {
        if (m_alive)
            *m_alive = false;
    }
    jsValue* duplicate(jsValue* owner) { return this; }
    static  void init(jsTypeLibrary& typeLibrary)
    {
        jsClass<MyValue> cls(typeLibrary, "MyValue");
        cls.Constructor<bool&>();
    }
private:
    bool* m_alive;
};

TEST(jsValueTests, TestWrapToSharedPtr_ThenDeleteSharedPtr_ObjectDeleted)
{
    DNVS::MoFa::Services::ScopedServiceProvider sp;
    auto scriptingService = ScriptingServiceFactory().CreateScriptingService();
    MyValue::init(jsStack::stack()->GetJsTypeLibrary());
    bool alive = false;
    std::shared_ptr<MyValue> val(new MyValue(alive));
    ASSERT_TRUE(alive);
    DNVS::MoFa::Reflection::Objects::Object o(jsStack::stack()->GetTypeLibrary(), val);
    MyValue* myValue = o.As<MyValue*>();
    o.Reset();
    val.reset();
    ASSERT_FALSE(alive);
}