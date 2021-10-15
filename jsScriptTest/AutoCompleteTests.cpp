#include <Services/ScopedServiceProvider.h>
#include <GoogleTest/CppunitAdapter.h>
#include <Reflection/TypeLibraries/TypeLibraryFactory.h>
#include "Reflection/Classes/Class.h"
#include "jsScript/jsStack.h"
#include "jsScript/jsTypeLibrary.h"
#include "Scripting/Scripting.h"
#include "Scripting/IAutoCompletion.h"
#include "ScriptingServiceFactory.h"

using namespace DNVS::MoFa::Scripting;

class AutoCompleteTests : public ::testing::Test
{
public:
    AutoCompleteTests() 
    {
    }
    virtual void SetUp() override
    {
        m_scriptingService = ScriptingServiceFactory().CreateScriptingService();
        m_sp.RegisterService(m_scriptingService);
    }

protected:
    ScriptingServicePointer m_scriptingService; 
    DNVS::MoFa::Services::ScopedServiceProvider m_sp;
};

TEST_F(AutoCompleteTests,TestOpeningParenthesis)
{
    CPPUNIT_ASSERT_NO_THROW(TryAutoComplete("("));
}

TEST_F(AutoCompleteTests,TestOpeningPeriod)
{
    CPPUNIT_ASSERT_NO_THROW(TryAutoComplete("."));
}

TEST_F(AutoCompleteTests,TestOpeningDoublePeriod)
{
    CPPUNIT_ASSERT_NO_THROW(TryAutoComplete(".."));
}

TEST_F(AutoCompleteTests,TestIncorrectOrderOfParenthesis)
{
    CPPUNIT_ASSERT_NO_THROW(TryAutoComplete("a= b)("));
}

TEST_F(AutoCompleteTests,TestOpeningArrayClause)
{
    CPPUNIT_ASSERT_NO_THROW(TryAutoComplete("["));
}

TEST_F(AutoCompleteTests,TestIncorrectOrderOfArrayClause)
{
    CPPUNIT_ASSERT_NO_THROW(TryAutoComplete("a= b]["));
}

TEST_F(AutoCompleteTests,TestClosingArrayClause)
{
    CPPUNIT_ASSERT_NO_THROW(TryAutoComplete("]"));
}

TEST_F(AutoCompleteTests,TestClosingParenthesis)
{
    CPPUNIT_ASSERT_NO_THROW(TryAutoComplete(")"));
}

class TestAutoCompleteClass {
public:
    TestAutoCompleteClass(int, int) {}
    TestAutoCompleteClass(double) {}

    void Test(int a) {}
    const TestAutoCompleteClass& GetSelf() const { return *this; }
    void SetSelf(const TestAutoCompleteClass& other) {}
};
void DoReflect(DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer typeLibrary, TestAutoCompleteClass**)
{
    using namespace DNVS::MoFa::Reflection::Classes;
    Class<TestAutoCompleteClass> cls(typeLibrary, "TestAutoCompleteClass");
    cls.Constructor<int, int>().AddSignature("a", "b");
    cls.Constructor<double>().AddSignature("c");
    cls.Function("Test", &TestAutoCompleteClass::Test)
        .AddSignature("a");
    cls.SetGet("Self", &TestAutoCompleteClass::SetSelf, &TestAutoCompleteClass::GetSelf);
}
TEST_F(AutoCompleteTests, TestParseReflectedConstructor)
{
    DNVS::MoFa::Reflection::Reflect<TestAutoCompleteClass>(jsStack::stack()->GetTypeLibrary());
    auto autoComplete = TryAutoComplete("TestAutoCo");
    ASSERT_NE(nullptr, autoComplete);
    EXPECT_EQ(1, autoComplete->GetVariableSet().size());
}

TEST_F(AutoCompleteTests, TestParseReflectedConstructorArguments)
{
    DNVS::MoFa::Reflection::Reflect<TestAutoCompleteClass>(jsStack::stack()->GetTypeLibrary());
    auto autoComplete = TryAutoComplete("TestAutoCompleteClass(");
    ASSERT_NE(nullptr, autoComplete);
    ASSERT_EQ(2, autoComplete->GetFunctionSet().size());
    EXPECT_EQ("TestAutoCompleteClass * TestAutoCompleteClass(double c)", *autoComplete->GetFunctionSet().begin());
}

TEST_F(AutoCompleteTests, TestParseMemberFunction)
{
    DNVS::MoFa::Reflection::Reflect<TestAutoCompleteClass>(jsStack::stack()->GetTypeLibrary());
    Execute("a = TestAutoCompleteClass(5);");
    auto autoComplete = TryAutoComplete("a.Te");
    ASSERT_NE(nullptr, autoComplete);
    ASSERT_EQ(1, autoComplete->GetVariableSet().size());
    autoComplete = TryAutoComplete("a.Test(");
    ASSERT_EQ(1, autoComplete->GetFunctionSet().size());
    EXPECT_EQ("void Test(int a)", *autoComplete->GetFunctionSet().begin());
}

TEST_F(AutoCompleteTests, TestParseNestedFunction)
{
    typedef double (AutoCompleteTests::*Fn)(double, double) const;
    static_assert(2 == DNVS::MoFa::TypeUtilities::FunctionTraits<Fn>::Arity - std::is_member_function_pointer_v<Fn> ? 1 : 0, "Expected 2, found 1");
    DNVS::MoFa::Reflection::Reflect<TestAutoCompleteClass>(jsStack::stack()->GetTypeLibrary());
    Execute("a = TestAutoCompleteClass(5);");
    auto autoComplete = TryAutoComplete("a.Self.Te");
    ASSERT_NE(nullptr, autoComplete);
    ASSERT_EQ(1, autoComplete->GetVariableSet().size());
    autoComplete = TryAutoComplete("a.Self.Test(");
    ASSERT_EQ(1, autoComplete->GetFunctionSet().size());
}

TEST_F(AutoCompleteTests, TestParseSetProperty)
{
    DNVS::MoFa::Reflection::Reflect<TestAutoCompleteClass>(jsStack::stack()->GetTypeLibrary());
    Execute("a = TestAutoCompleteClass(5);");
    EXPECT_EQ(1, TryAutoComplete("a.Self = TestAutoCo")->GetVariableSet().size());
    EXPECT_EQ(1, TryAutoComplete("a.Self =TestAutoCo")->GetVariableSet().size());
}

TEST_F(AutoCompleteTests, TestParseJSFunction)
{
    Execute("MyMethod = function(a : number, b : number) : number {return a+b;};");
    EXPECT_EQ(1, TryAutoComplete("a.Self = MyMethod")->GetVariableSet().size());
    auto autoComplete = TryAutoComplete("a.Self = MyMethod(");
    EXPECT_EQ(1, autoComplete->GetFunctionSet().size());
    EXPECT_EQ("double MyMethod(double a, double b)", *autoComplete->GetFunctionSet().begin());
}