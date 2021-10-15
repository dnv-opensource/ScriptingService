#include "gtest\gtest.h"
#include <Services/ScopedServiceProvider.h>
#include "jsScript\jsClass.h"
#include "jsScript\jsScriptable.h"

#include "Units/Length.h"
#include "jsUnits/jsUnits.h"
#include "jsUnits/jsMassDensity.h"
#include "jsUnits/jsStress.h"
#include "jsUnits/jsForcePerUnitLength.h"
#include "jsUnits/jsForcePerUnitArea.h"
#include "Units/Stress.h"
#include "jsUnits/jsLength.h"
#include "jsUnits/jsForce.h"

#include "Scripting\As.h"

#include "Reflection/Members/Modifier.h"
#include "Reflection/Members/Arg.h"
#include "ScriptingServiceFactory.h"
#include "Reflection\Classes\Class.h"

using namespace DNVS::MoFa::Scripting;
using namespace DNVS::MoFa::Reflection;
using namespace Members;
using DNVS::MoFa::Reflection::Variants::Variant;

void TestFunction()
{
    std::cout << "Help";
}
class ClassWithExplicitConstructor : public jsScriptable 
{
public:
    ClassWithExplicitConstructor(const jsAutomation& automation) : jsScriptable(automation) {}
    ClassWithExplicitConstructor(int a) : m_a(a) {}
    ~ClassWithExplicitConstructor() 
    {
        TestFunction();
    }
    static ClassWithExplicitConstructor* CustomConstructorFunction(int a)  {return new ClassWithExplicitConstructor(a); }
    void TestCall(const ClassWithExplicitConstructor& input) {}
    static void init(jsTypeLibrary& typeLibrary) 
    {
        jsClass<ClassWithExplicitConstructor> cls(typeLibrary, "ClassWithExplicitConstructor");
        cls.Constructor("ClassWithExplicitConstructor", CustomConstructorFunction, Explicit);
        cls.Function("TestCall", &ClassWithExplicitConstructor::TestCall);
    }
private:
    int m_a;
};

class jsClassTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
         m_scriptingService = ScriptingServiceFactory().CreateScriptingService();
         m_sp.RegisterService<TypeLibraries::ITypeLibrary>(m_scriptingService->GetTypeLibrary());
    }

    DNVS::MoFa::Services::ScopedServiceProvider m_sp;
    ScriptingServicePointer m_scriptingService;
};

TEST_F(jsClassTests, AddExplicitConstructor_NotUsedInImplicitConversion)
{
    ClassWithExplicitConstructor::init(jsStack::stack()->GetJsTypeLibrary());
    m_scriptingService->Execute("a=ClassWithExplicitConstructor(4);");
    ASSERT_THROW(m_scriptingService->Execute("a.TestCall(5);"), std::exception);
}

class ClassWithRegularConstructor : public jsScriptable 
{
public:
    ClassWithRegularConstructor(const jsAutomation& automation) : jsScriptable(automation) {}
    ClassWithRegularConstructor(int a) : m_a(a) {}
    void TestCall(const ClassWithRegularConstructor& input) {}
    static void init(jsTypeLibrary& typeLibrary) 
    {
        jsClass<ClassWithRegularConstructor> cls(typeLibrary, "ClassWithRegularConstructor");
        cls.Constructor<int>();
        cls.Function("TestCall", &ClassWithRegularConstructor::TestCall);
    }
private:
    int m_a;
};
TEST_F(jsClassTests, AddRegularConstructor_UsedInImplicitConversion)
{
    ClassWithRegularConstructor::init(jsStack::stack()->GetJsTypeLibrary());
    m_scriptingService->Execute("a=ClassWithRegularConstructor(4);");
    ASSERT_NO_THROW(m_scriptingService->Execute("a.TestCall(5);"));
}

class ClassWithStaticFunction : public jsScriptable 
{
public:
    ClassWithStaticFunction(const jsAutomation& automation) : jsScriptable(automation) {}
    ClassWithStaticFunction() {}
    static int TestCall() {return 42; }
    static void init(jsTypeLibrary& typeLibrary) 
    {
        jsClass<ClassWithStaticFunction> cls(typeLibrary, "ClassWithStaticFunction");
        cls.StaticFunction("TestCall", TestCall);
    }
};
TEST_F(jsClassTests, AddStaticFunction)
{
    ClassWithStaticFunction::init(jsStack::stack()->GetJsTypeLibrary());
    ASSERT_NO_THROW(m_scriptingService->Execute("a=TestCall();"));
    ASSERT_EQ(42, m_scriptingService->Execute("TestCall();").As<int>());
}

class ExtensionClass : public jsScriptable 
{
public:
    ExtensionClass() {}
    ExtensionClass(const jsAutomation& automation) : jsScriptable(automation) {}
    ExtensionClass(ClassWithRegularConstructor* cls) : m_static(cls) {}
    operator ClassWithRegularConstructor*() const {return m_static; }
    int TestCall2() {return 42; }
    static void init(jsTypeLibrary& typeLibrary) 
    {
        jsClass<ExtensionClass> cls(typeLibrary, "ExtensionClass");
        cls.Function("TestCall2", &ExtensionClass::TestCall2);
        cls.unite(jsType<ClassWithRegularConstructor>());
    }
private:
    mofa::ref<ClassWithRegularConstructor> m_static;
};
TEST_F(jsClassTests, UniteClassTight)
{
    ClassWithRegularConstructor::init(jsStack::stack()->GetJsTypeLibrary());
    ExtensionClass::init(jsStack::stack()->GetJsTypeLibrary());
    ASSERT_NO_THROW(m_scriptingService->Execute("a=ClassWithRegularConstructor(3);"));
    ASSERT_NO_THROW(m_scriptingService->Execute("b=a.TestCall2();"));
    ASSERT_EQ(42, m_scriptingService->Execute("b;").As<int>());
}

TEST_F(jsClassTests, ConvertJavascriptFunctionToJsFunction)
{
    ASSERT_NO_THROW(m_scriptingService->Execute("a=function(x, y, z) {};").As<MemberPointer>());
}

TEST_F(jsClassTests, ScriptMassDensity)
{
    m_scriptingService->Execute("Maxwidth=12m;");
    m_scriptingService->Execute("BilgeRadius=2m;");
    ASSERT_NO_THROW(m_scriptingService->Execute("Maxwidth - BilgeRadius;"));
    ASSERT_EQ(DNVS::MoFa::Units::Length(10), m_scriptingService->Execute("Maxwidth - BilgeRadius;").As<DNVS::MoFa::Units::Length>());
}

TEST_F(jsClassTests, StringifyFunctionWithUnitExpression)
{
    auto val = m_scriptingService->Execute("a=function() {var a=5kg*m/s^2;};");
    MemberPointer fn = val.As<MemberPointer>();
    ASSERT_EQ(fn->Format(IMember::FormatType::Function), "function()\r\n{\r\nvar a = 5 kg*m/s^2;\r\n}");
    ASSERT_NO_THROW(m_scriptingService->Execute("a();"));
}

TEST_F(jsClassTests, ScriptForceAsMassLengthPerTimeSquare)
{
    ASSERT_NO_THROW(m_scriptingService->Execute("a=5 kg*m/s^2;"));
}

TEST_F(jsClassTests, jsLengthMinusLength)
{
    ASSERT_NO_THROW(m_scriptingService->Execute("Length(4)-4m;"));
}

TEST_F(jsClassTests, LengthMinusJsLength)
{
    ASSERT_NO_THROW(m_scriptingService->Execute("4m-Length(4);"));
}

TEST_F(jsClassTests, StringToLong)
{
    ClassWithRegularConstructor::init(jsStack::stack()->GetJsTypeLibrary());
    ASSERT_NO_THROW(m_scriptingService->Execute("a=ClassWithRegularConstructor('45');"));
}

TEST_F(jsClassTests, ToBoolean)
{

    ClassWithRegularConstructor::init(jsStack::stack()->GetJsTypeLibrary());
    ASSERT_EQ(true, m_scriptingService->Execute("true;").As<bool>());
    ASSERT_EQ(false, m_scriptingService->Execute("false;").As<bool>());
}

TEST_F(jsClassTests, EmptyStringToBoolean)
{

    ClassWithRegularConstructor::init(jsStack::stack()->GetJsTypeLibrary());
    ASSERT_EQ(false, m_scriptingService->Execute("'';").As<bool>());
}

TEST_F(jsClassTests, ParseMassDensity)
{
    ASSERT_NEAR(jsMassDensity(5), m_scriptingService->Execute("5 kg/m^3;").As<jsMassDensity>(), 1e-6);
}

TEST_F(jsClassTests, ConvertStressToForcePerArea)
{
    jsStress a(0);
    auto value = As<Variant>(&a);
    ASSERT_NO_THROW(As<jsForcePerUnitArea>(value , m_scriptingService));
}

TEST_F(jsClassTests, ConvertjsStressToString)
{
    jsStress a(5);

    auto value = As<Variant>(&a);
    std::string result;
    ASSERT_NO_THROW(result = As<std::string>(value, m_scriptingService));
    ASSERT_EQ("5 Pa", result);
}

TEST_F(jsClassTests, ConvertjsStressTojsValue)
{
    jsStress a(5);

    ASSERT_NO_THROW(As<Variant>(a));
}

TEST_F(jsClassTests, intToJsValue)
{
    auto value = As<Variant>(5);
    ASSERT_EQ(5, As<int>(value, m_scriptingService));
}

TEST_F(jsClassTests, unsignedintToJsValue)
{
    auto value = As<Variant,unsigned int>(5);
    ASSERT_EQ(5, As<unsigned int>(value, m_scriptingService));
}

TEST_F(jsClassTests, CreateFunctionAndInvoke)
{
    m_scriptingService->Execute("function a(x) {return x-5m;}");
    ASSERT_NO_THROW(m_scriptingService->Execute("a(Length(8));"));
}

TEST_F(jsClassTests, StringToLong_CheckValue)
{
    ASSERT_EQ(1256, m_scriptingService->Execute("'Bm1256'.substring(2,100);").As<long>());
}

TEST_F(jsClassTests, ScriptForceAsMassLengthTimesTimeToThePowerOfNegativeTwo)
{
    ASSERT_NO_THROW(m_scriptingService->Execute("a=Force(5 kg*m*s^-2);"));
}

TEST_F(jsClassTests, OneOverTimeConvertibleToFrequency)
{
    ASSERT_NO_THROW(m_scriptingService->Execute("a=Frequency(1/5s);"));
    ASSERT_NO_THROW(m_scriptingService->Execute("a=Frequency(1/Time(5s));"));
    /*ASSERT_NO_THROW(*/m_scriptingService->Execute("a=Frequency(1/(1/Frequency(5)));")/*)*/;
}

TEST_F(jsClassTests, VelocityTimesFrequency)
{
    ASSERT_NO_THROW(m_scriptingService->Execute("a=1m;"));
    ASSERT_NO_THROW(m_scriptingService->Execute("c=1s;"));
    ASSERT_NO_THROW(m_scriptingService->Execute("f=1/c;"));
    ASSERT_NO_THROW(m_scriptingService->Execute("g=a*f;"));
    ASSERT_NO_THROW(m_scriptingService->Execute("h=g*f;"));
}

TEST_F(jsClassTests, LengthTimesForcePerLengthByGravity)
{
    ASSERT_NO_THROW(m_scriptingService->Execute("fpl=ForcePerUnitLength(1);"));
    ASSERT_NO_THROW(m_scriptingService->Execute("g=9.81m/s2;"));
    ASSERT_NO_THROW(m_scriptingService->Execute("l=1m;"));
    ASSERT_NO_THROW(m_scriptingService->Execute("r=l*(fpl/g);"));
}

TEST_F(jsClassTests, LengthTimesForcePerLengthByGravityByInteger)
{
    ASSERT_NO_THROW(m_scriptingService->Execute("fpl=ForcePerUnitLength(1);"));
    ASSERT_NO_THROW(m_scriptingService->Execute("g=9.81m/s2;"));
    ASSERT_NO_THROW(m_scriptingService->Execute("l=1m;"));
    ASSERT_NO_THROW(m_scriptingService->Execute("r=l*(fpl/g);"));
    ASSERT_NO_THROW(m_scriptingService->Execute("s=r/5;"));
}


TEST_F(jsClassTests, LengthTimesForcePerLengthByGravityByLength)
{
    ASSERT_NO_THROW(m_scriptingService->Execute("fpl=ForcePerUnitLength(1);"));
    ASSERT_NO_THROW(m_scriptingService->Execute("g=9.81m/s2;"));
    ASSERT_NO_THROW(m_scriptingService->Execute("l=1m;"));
    ASSERT_NO_THROW(m_scriptingService->Execute("r=l*(fpl/g);"));
    ASSERT_NO_THROW(m_scriptingService->Execute("s=r/5m;"));
}

TEST_F(jsClassTests, LengthTimesForcePerLengthByGravityByLengthToDouble)
{
    ASSERT_NO_THROW(m_scriptingService->Execute("fpl=ForcePerUnitLength(1);"));
    ASSERT_NO_THROW(m_scriptingService->Execute("g=9.81m/s2;"));
    ASSERT_NO_THROW(m_scriptingService->Execute("l=1m;"));
    ASSERT_NO_THROW(m_scriptingService->Execute("r=l*(fpl/g);"));
    ASSERT_NO_THROW(m_scriptingService->Execute("s=r/5m;"));
    ASSERT_NO_THROW(m_scriptingService->Execute("b=s.toDouble();"));
}

TEST_F(jsClassTests, LengthTimesForcePerLength)
{
    ASSERT_NO_THROW(m_scriptingService->Execute("fpl=ForcePerUnitLength(1);"));
    ASSERT_NO_THROW(m_scriptingService->Execute("l=1m;"));
    ASSERT_NO_THROW(m_scriptingService->Execute("r=l*(fpl);"));
}

TEST_F(jsClassTests, LengthByLengthPerTime)
{
    ASSERT_NO_THROW(m_scriptingService->Execute("l=Length(1);"));
    ASSERT_NO_THROW(m_scriptingService->Execute("l2=1m;"));
    ASSERT_NO_THROW(m_scriptingService->Execute("r=l2/(l/1s);"));
}

TEST_F(jsClassTests, LengthjsLessThanLength)
{
    EXPECT_EQ(true, m_scriptingService->Execute("Length(4)<5m;").As<bool>());
}


TEST_F(jsClassTests, NmPowerNegativeOne)
{
    auto object = m_scriptingService->Execute("1Nm^-1;");
    jsQuantity quantity = object.As<jsQuantity>();
    EXPECT_EQ("N/m", quantity.GetQuantity().GetUnit().GetUnitName());
}

TEST_F(jsClassTests, ParseInch)
{
    EXPECT_NO_THROW(m_scriptingService->Execute("a=1in;"));
}

TEST_F(jsClassTests, UnitToDouble)
{
    m_scriptingService->Execute("a=1N;");
    EXPECT_NO_THROW(m_scriptingService->Execute("a.toDouble();"));
}

TEST_F(jsClassTests, CompareLengthToQuantity)
{
    EXPECT_NO_THROW(m_scriptingService->Execute("1m<Length(2);"));
    EXPECT_TRUE(m_scriptingService->Execute("1m<Length(2);").As<bool>());
    EXPECT_FALSE(m_scriptingService->Execute("1m>Length(2);").As<bool>());
    EXPECT_TRUE(m_scriptingService->Execute("1m>=Length(1);").As<bool>());
    EXPECT_FALSE(m_scriptingService->Execute("2m<=Length(1);").As<bool>());
    EXPECT_FALSE(m_scriptingService->Execute("2m!=Length(2);").As<bool>());
    EXPECT_TRUE(m_scriptingService->Execute("2m==Length(2);").As<bool>());
}

TEST_F(jsClassTests, AddLengths)
{
    EXPECT_NO_THROW(m_scriptingService->Execute("1m+Length(2);"));
    EXPECT_NEAR(3., m_scriptingService->Execute("1m+Length(2);").As<jsLength>(), 1e-6);
    EXPECT_NEAR(-1., m_scriptingService->Execute("1N-Force(2);").As<jsForce>(), 1e-6);
}

TEST_F(jsClassTests, LessLengths)
{
    EXPECT_TRUE(m_scriptingService->Execute("1m<2m;").As<bool>());
    EXPECT_TRUE(m_scriptingService->Execute("!(1m>2m);").As<bool>());
}

TEST_F(jsClassTests, OperatorsOnDouble)
{
    EXPECT_TRUE(m_scriptingService->Execute("1m.toDouble()<2;").As<bool>());
    EXPECT_NEAR(3.1, m_scriptingService->Execute("1.1m.toDouble()+2;").As<double>(), 1e-6);
    EXPECT_NEAR(2.1, m_scriptingService->Execute("1m.toDouble()*2.1;").As<double>(), 1e-6);
    EXPECT_NEAR(2.2, m_scriptingService->Execute("1.1m.toDouble()*2;").As<double>(), 1e-6);
}

TEST_F(jsClassTests, toDoubleAsPropertyShouldNotWork)
{
    EXPECT_THROW(m_scriptingService->Execute("1m.toDouble;").As<double>(), std::exception);
}

TEST_F(jsClassTests, TestArrayNotation)
{
    m_scriptingService->Execute("a=Array();");
    EXPECT_NO_THROW(m_scriptingService->Execute("a[0]=5;"));
}

using namespace DNVS::MoFa::Reflection::Members;

class ClassWithFunctionAndSignature : public jsScriptable
{
public:
    ClassWithFunctionAndSignature(const jsAutomation& automation) : jsScriptable(automation)
    {
    }

    ClassWithFunctionAndSignature()
    {
    }

    int Sum(int a, int b = 1)
    {
        return a + b;
    }

    static void init(jsTypeLibrary& typeLibrary)
    {
        jsClass<ClassWithStaticFunction> cls(typeLibrary, "ClassWithFunctionAndSignature");

        auto& fun =
            cls.Function("Sum", &ClassWithFunctionAndSignature::Sum)
                .AddSignature((Arg("a", "This is the first argument"), Arg("b", "This is the second argument")));

        ASSERT_STREQ("a", fun.argument(0)->varName().c_str());
        ASSERT_STREQ("This is the first argument", fun.argument(0)->docShort().c_str());
        ASSERT_STREQ("b", fun.argument(1)->varName().c_str());
        ASSERT_STREQ("This is the second argument", fun.argument(1)->docShort().c_str());
    }
};

TEST_F(jsClassTests, AfterAddingSignatureVariablesNamesAndDocumentationStringsAreTheSameAsPassed)
{
    ClassWithFunctionAndSignature::init(jsStack::stack()->GetJsTypeLibrary());
}
