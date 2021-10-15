#include <jsScript/jsScriptable.h>
#include <jsScript/jsClass.h>

#include <Services/ScopedServiceProvider.h>
#include "GoogleTest\CppunitAdapter.h"
#include "Scripting\As.h"
#include "ScriptingServiceFactory.h"


using namespace DNVS::MoFa::Scripting;
class FirstClass : public jsScriptable
{
public:
    FirstClass(int a) : m_a(a) {}
    FirstClass(const jsAutomation& automation) : jsScriptable(automation) {}
    int GetValue() {return m_a;}
    static void init(jsTypeLibrary& typeLibrary,bool enable)
    {
        jsClass<FirstClass> cls(typeLibrary,"TestClass");
        if(cls.reinit()) return;
        cls.enableScripting(enable);
        cls.Constructor<int>();
        cls.Function("GetValue",&FirstClass::GetValue);
    }
private:
    int m_a;
};
class SecondClass : public jsScriptable
{
public:
    SecondClass(int a) : m_a(a*2) {}
    SecondClass(const jsAutomation& automation) : jsScriptable(automation) {}
    int GetValue() {return m_a;}
    static void init(jsTypeLibrary& typeLibrary,bool enable)
    {
        jsClass<SecondClass> cls(typeLibrary,"TestClass");
        if(cls.reinit()) return;
        cls.enableScripting(enable);
        cls.Constructor<int>();
        cls.Function("GetValue",&SecondClass::GetValue);
    }
private:
    int m_a;
};

class ConditionalClassRegistration : public ::testing::Test
{
protected:
    void SetUp() override
    {
         m_scriptingService = ScriptingServiceFactory().CreateScriptingService();
    }

    DNVS::MoFa::Services::ScopedServiceProvider m_sp;
    ScriptingServicePointer m_scriptingService;
};

TEST_F(ConditionalClassRegistration,ReserveWord)
{
    FirstClass::init(jsStack::stack()->GetJsTypeLibrary(),false);
    CPPUNIT_ASSERT(jsStack::stack()->GetJsTypeLibrary().isReservedWord("TestClass"));
    EXPECT_THROW(m_scriptingService->Execute("TestClass=2;"),std::runtime_error);
}

TEST_F(ConditionalClassRegistration,RegisterDuplicate_SecondOverrideFirst)
{
    FirstClass::init(jsStack::stack()->GetJsTypeLibrary(),false);
    SecondClass::init(jsStack::stack()->GetJsTypeLibrary(),true);
    int result=m_scriptingService->Execute("TestClass(21).GetValue();").As<int>();
    CPPUNIT_ASSERT_EQUAL(42,result);
}

TEST_F(ConditionalClassRegistration,RegisterDuplicate_FirstAndSecond_SecondWins)
{
    FirstClass::init(jsStack::stack()->GetJsTypeLibrary(),true);
    SecondClass::init(jsStack::stack()->GetJsTypeLibrary(),false);
    int result=m_scriptingService->Execute("TestClass(42).GetValue();").As<int>();
    CPPUNIT_ASSERT_EQUAL(42,result);
}
