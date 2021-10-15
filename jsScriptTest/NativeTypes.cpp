#include "gtest\gtest.h"
#include <Services/ScopedServiceProvider.h>
#include "jsScript\jsTypeLibrary.h"
#include "Reflection\TypeLibraries\TypeLibraryFactory.h"
#include "Reflection\Classes\ClassArgumentPack.h"
#include "Reflection\Classes\Class.h"
#include "Reflection\Containers\ReflectVector.h"
#include <numeric>
#include "jsScript\jsConversions.h"
#include "jsScript\jsScriptable.h"
#include "jsScript\jsClass.h"
#include "ScriptingServiceFactory.h"
#include "jsScript\IScriptingPropertyService.h"
#include "jsUnits\jsLength.h"
#include <chrono>
#include "Units/Pressure.h"
#include "Units\Runtime\DynamicQuantity.h"

using namespace DNVS::MoFa::Reflection;
using namespace DNVS::MoFa::Scripting;
using namespace DNVS::MoFa::Services;
using namespace DNVS::MoFa::Units;

class NativeTypes : public ::testing::Test
{
public:
    void SetUp()
    {
        scriptingService = ScriptingServiceFactory().CreateScriptingService();
        sp.RegisterService(scriptingService);
        sp.RegisterService(scriptingService->GetTypeLibrary());
    }
    std::string GetBoolTypeString()
    { 
        return "bool";
    }
protected:
    ScopedServiceProvider sp;
    ScriptingServicePointer scriptingService;
};

TEST_F(NativeTypes, RelativeCompareString)
{
    scriptingService->Execute("a='Hei';");
    scriptingService->Execute("b='Hopp';");
    EXPECT_TRUE(scriptingService->Execute("a<b;").As<bool>());
    EXPECT_TRUE(scriptingService->Execute("a>=a;").As<bool>());
    EXPECT_FALSE(scriptingService->Execute("a>b;").As<bool>());

    scriptingService->Execute("a='12';");
    scriptingService->Execute("b=4;");
    EXPECT_TRUE(scriptingService->Execute("a>b;").As<bool>());
    EXPECT_TRUE(scriptingService->Execute("b<a;").As<bool>());

    scriptingService->Execute("a=12;");
    scriptingService->Execute("b='4';");
    EXPECT_TRUE(scriptingService->Execute("a>b;").As<bool>());
    EXPECT_TRUE(scriptingService->Execute("b<a;").As<bool>());
}

TEST_F(NativeTypes, StringTests)
{
    scriptingService->Execute("a='Dette er en test';");
    EXPECT_EQ(16, scriptingService->Execute("a.length;").As<int>());
    EXPECT_EQ("e", scriptingService->Execute("a.charAt(4);").As<std::string>());
    EXPECT_EQ('e', scriptingService->Execute("a.charCodeAt(4);").As<char>());
    EXPECT_EQ("Dette er en test ja 4", scriptingService->Execute("a.concat(' ja ', 4);").As<std::string>());
    EXPECT_EQ(6, scriptingService->Execute("a.indexOf('er');").As<int>());
    EXPECT_EQ(-1, scriptingService->Execute("a.indexOf('ers');").As<int>());
    EXPECT_EQ(12, scriptingService->Execute("a.indexOf('te',6);").As<int>());
    EXPECT_EQ(12, scriptingService->Execute("a.lastIndexOf('te');").As<int>());
    EXPECT_EQ(3, scriptingService->Execute("a.lastIndexOf('te',6);").As<int>());
    EXPECT_EQ("e er en test", scriptingService->Execute("a.slice(4);").As<std::string>());
    EXPECT_EQ("e e", scriptingService->Execute("a.slice(4,7);").As<std::string>());
    EXPECT_EQ("en test", scriptingService->Execute("a.slice(-7);").As<std::string>());
    EXPECT_EQ("n tes", scriptingService->Execute("a.slice(-6,-1);").As<std::string>());
    EXPECT_EQ("bcdef", scriptingService->Execute("'abcdef'.replace('a','');").As<std::string>());
    EXPECT_EQ(4, scriptingService->Execute("a.split(' ').length;").As<int>());
    EXPECT_EQ(3, scriptingService->Execute("a.split(' ', 3).length;").As<int>());
    EXPECT_EQ("en", scriptingService->Execute("a.split(' ')[2];").As<std::string>());
    EXPECT_EQ("en", scriptingService->Execute("a.split(' ',3)[2];").As<std::string>());
    EXPECT_EQ(16, scriptingService->Execute("a.split().length;").As<int>());
    EXPECT_EQ(8, scriptingService->Execute("a.split('',8).length;").As<int>());
    EXPECT_EQ("e", scriptingService->Execute("a.split('',8)[4];").As<std::string>());
    EXPECT_EQ("Dette er en test", scriptingService->Execute("a.toString();").As<std::string>());
    EXPECT_EQ("Detto er en tost", scriptingService->Execute("a.replace('te','to');").As<std::string>());
    EXPECT_EQ(7, scriptingService->Execute("a.search('r e');").As<int>());
    EXPECT_EQ("dette er en test", scriptingService->Execute("a.toLowerCase();").As<std::string>());
    EXPECT_EQ("DETTE ER EN TEST", scriptingService->Execute("a.toUpperCase();").As<std::string>());
    EXPECT_EQ("tte", scriptingService->Execute("a.substring(2,2+3);").As<std::string>());
    EXPECT_EQ("tte", scriptingService->Execute("a.substring(2+3,2);").As<std::string>());
    EXPECT_EQ("tte", scriptingService->Execute("a.substr(2,3);").As<std::string>());
}

TEST_F(NativeTypes, ArrayTests)
{
    scriptingService->Execute("a=Array(1,2);");
    scriptingService->Execute("a[0] = 4;");
    EXPECT_EQ(4, scriptingService->Execute("a[0];").As<int>());
    scriptingService->Execute("a[12] = 'Hello world';");
    EXPECT_EQ(13, scriptingService->Execute("a.length;").As<size_t>());
    EXPECT_EQ("Hello world", scriptingService->Execute("a.pop();").As<std::string>());
    EXPECT_EQ(12, scriptingService->Execute("a.length;").As<size_t>());
    EXPECT_EQ(13, scriptingService->Execute("a.push('Bye world');").As<size_t>());
    EXPECT_EQ(13, scriptingService->Execute("a.length;").As<size_t>());
    EXPECT_EQ("Bye world", scriptingService->Execute("a[12];").As<std::string>());
    AutoReflect<std::vector<int>>::Reflect(scriptingService->GetTypeLibrary());
    scriptingService->Execute("b=Array(3,5);");
    EXPECT_EQ((std::vector<int>{3, 5}),scriptingService->Execute("b;").As<std::vector<int>>());
}

TEST_F(NativeTypes, ArrayReverse)
{
    scriptingService->Execute("a=Array(1,2);");
    scriptingService->Execute("a.reverse();");
    EXPECT_EQ(2, scriptingService->Execute("a[0];").As<int>());
    EXPECT_EQ(1, scriptingService->Execute("a[1];").As<int>());
}

TEST_F(NativeTypes, ArrayShift)
{
    scriptingService->Execute("a=Array(1,2);");
    EXPECT_EQ(1, scriptingService->Execute("a.shift();").As<int>());
    EXPECT_EQ(1, scriptingService->Execute("a.length;").As<int>());
    EXPECT_EQ(2, scriptingService->Execute("a[0];").As<int>());
}

TEST_F(NativeTypes, ArrayUnshift)
{
    scriptingService->Execute("a=Array(1,2);");
    EXPECT_EQ(5, scriptingService->Execute("a.unshift(3,4,5);").As<int>());
    EXPECT_EQ(3, scriptingService->Execute("a[0];").As<int>());
    EXPECT_EQ(4, scriptingService->Execute("a[1];").As<int>());
    EXPECT_EQ(5, scriptingService->Execute("a[2];").As<int>());
    EXPECT_EQ(1, scriptingService->Execute("a[3];").As<int>());
    EXPECT_EQ(2, scriptingService->Execute("a[4];").As<int>());
}

TEST_F(NativeTypes, ArraySplice)
{
    scriptingService->Execute("a=Array(1,2,3,4,5);");
    scriptingService->Execute("b = a.splice(2,2);");
    EXPECT_EQ(2, scriptingService->Execute("b.length;").As<int>());
    EXPECT_EQ(3, scriptingService->Execute("b[0];").As<int>());
    EXPECT_EQ(4, scriptingService->Execute("b[1];").As<int>());
    EXPECT_EQ(3, scriptingService->Execute("a.length;").As<int>());
    EXPECT_EQ(1, scriptingService->Execute("a[0];").As<int>());
    EXPECT_EQ(2, scriptingService->Execute("a[1];").As<int>());
    EXPECT_EQ(5, scriptingService->Execute("a[2];").As<int>());
}

TEST_F(NativeTypes, ArraySplice_OneArgument)
{
    scriptingService->Execute("a=Array(1,2,3,4,5);");
    scriptingService->Execute("b = a.splice(2);");
    EXPECT_EQ(3, scriptingService->Execute("b.length;").As<int>());
    EXPECT_EQ(3, scriptingService->Execute("b[0];").As<int>());
    EXPECT_EQ(4, scriptingService->Execute("b[1];").As<int>());
    EXPECT_EQ(5, scriptingService->Execute("b[2];").As<int>());
    EXPECT_EQ(2, scriptingService->Execute("a.length;").As<int>());
    EXPECT_EQ(1, scriptingService->Execute("a[0];").As<int>());
    EXPECT_EQ(2, scriptingService->Execute("a[1];").As<int>());
}

TEST_F(NativeTypes, ArraySplice_AddItems)
{
    scriptingService->Execute("a=Array(1,2,3,4,5);");
    scriptingService->Execute("b = a.splice(2,2,8,7,9,'Hello');");
    EXPECT_EQ(2, scriptingService->Execute("b.length;").As<int>());
    EXPECT_EQ(3, scriptingService->Execute("b[0];").As<int>());
    EXPECT_EQ(4, scriptingService->Execute("b[1];").As<int>());
    EXPECT_EQ(7, scriptingService->Execute("a.length;").As<int>());
    EXPECT_EQ(1, scriptingService->Execute("a[0];").As<int>());
    EXPECT_EQ(2, scriptingService->Execute("a[1];").As<int>());
    EXPECT_EQ(8, scriptingService->Execute("a[2];").As<int>());
    EXPECT_EQ(7, scriptingService->Execute("a[3];").As<int>());
    EXPECT_EQ(9, scriptingService->Execute("a[4];").As<int>());
    EXPECT_EQ("Hello", scriptingService->Execute("a[5];").As<std::string>());
    EXPECT_EQ(5, scriptingService->Execute("a[6];").As<int>());
}

TEST_F(NativeTypes, ArrayJoin)
{
    scriptingService->Execute("a=Array(1,2,3,4,'Hello');");
    EXPECT_EQ("1,2,3,4,Hello", scriptingService->Execute("a.join();").As<std::string>());
    EXPECT_EQ("1, 2, 3, 4, Hello", scriptingService->Execute("a.join(', ');").As<std::string>());
    EXPECT_EQ("1234Hello", scriptingService->Execute("a.join('');").As<std::string>());
    EXPECT_EQ("1 + 2 + 3 + 4 + Hello", scriptingService->Execute("a.join(' + ');").As<std::string>());
}
TEST_F(NativeTypes, ArraySlice)
{
    scriptingService->Execute("a=Array(1,2,3,4,5);");
    scriptingService->Execute("b = a.slice();");
    EXPECT_EQ(5, scriptingService->Execute("b.length;").As<int>());
    EXPECT_EQ(1, scriptingService->Execute("b[0];").As<int>());
    EXPECT_EQ(2, scriptingService->Execute("b[1];").As<int>());
    EXPECT_EQ(3, scriptingService->Execute("b[2];").As<int>());
    EXPECT_EQ(4, scriptingService->Execute("b[3];").As<int>());
    EXPECT_EQ(5, scriptingService->Execute("b[4];").As<int>());
    scriptingService->Execute("b = a.slice(3);");
    EXPECT_EQ(2, scriptingService->Execute("b.length;").As<int>());
    EXPECT_EQ(4, scriptingService->Execute("b[0];").As<int>());
    EXPECT_EQ(5, scriptingService->Execute("b[1];").As<int>());
    scriptingService->Execute("b = a.slice(2,4);");
    EXPECT_EQ(2, scriptingService->Execute("b.length;").As<int>());
    EXPECT_EQ(3, scriptingService->Execute("b[0];").As<int>());
    EXPECT_EQ(4, scriptingService->Execute("b[1];").As<int>());
}

TEST_F(NativeTypes, UndefinedElementsTests)
{
    //Old script engine handles this incorrectly
    scriptingService->Execute("a=Array(1,2);");
    scriptingService->Execute("a[3] = 33;");
    EXPECT_TRUE(scriptingService->Execute("a[2] == undefined;").As<bool>());
    EXPECT_TRUE(scriptingService->Execute("a[2] === undefined;").As<bool>());
    EXPECT_TRUE(scriptingService->Execute("a[1] != undefined;").As<bool>());
    EXPECT_TRUE(scriptingService->Execute("a[1] !== undefined;").As<bool>());
    scriptingService->Execute("var b;");
    EXPECT_TRUE(scriptingService->Execute("b == undefined;").As<bool>());
    EXPECT_TRUE(scriptingService->Execute("b === undefined;").As<bool>());
}

class Section {};
class TestClass {
public:
    Section* GetSection() const { return nullptr; }
};

void DoReflect(TypeLibraries::TypeLibraryPointer typeLibrary, Section**)
{
    using namespace Classes;
    Class<Section> cls(typeLibrary, "Section");
    cls.Operator(*This.Const == *This.Const);
    cls.Operator(*This.Const != *This.Const);
}
void DoReflect(TypeLibraries::TypeLibraryPointer typeLibrary, TestClass**)
{
    using namespace Classes;
    Class<TestClass,std::shared_ptr<TestClass>> cls(typeLibrary, "TestClass");
    cls.Constructor();
    cls.Get("Section", &TestClass::GetSection);
}

TEST_F(NativeTypes, NullElementsTests)
{
    Reflect<TestClass>(scriptingService->GetTypeLibrary());
    Reflect<Section>(scriptingService->GetTypeLibrary());
    scriptingService->Execute("a = null;");
    EXPECT_TRUE(scriptingService->Execute("a == null;").As<bool>());
    //Old script engine handles this incorrectly
    scriptingService->Execute("b = TestClass();");
    EXPECT_TRUE(scriptingService->Execute("b.Section == null;").As<bool>());
}

TEST_F(NativeTypes, GlobalFunctionsTest)
{
    EXPECT_EQ(42, scriptingService->Execute("eval('40+2;');").As<int>());
    EXPECT_EQ(25, scriptingService->Execute("parseInt('25');").As<int>());
    EXPECT_DOUBLE_EQ(25.55e44, scriptingService->Execute("parseFloat('25.55e44');").As<double>());
    //Old script engine handles this incorrectly
    EXPECT_DOUBLE_EQ(1234567891022333, scriptingService->Execute("parseInt('1234567891022333');").As<double>());
    EXPECT_TRUE(scriptingService->Execute("isNaN(parseInt('Balle'));").As<bool>());
    EXPECT_TRUE(scriptingService->Execute("isFinite(13.5);").As<bool>());
    EXPECT_FALSE(scriptingService->Execute("isFinite(Infinity);").As<bool>());
    EXPECT_FALSE(scriptingService->Execute("isFinite(-Infinity);").As<bool>());
    EXPECT_FALSE(scriptingService->Execute("isNaN(11);").As<bool>());
    EXPECT_TRUE(scriptingService->Execute("isNaN(NaN);").As<bool>());
    EXPECT_FALSE(scriptingService->Execute("isFinite(parseFloat('-Infinity'));").As<bool>());
}

TEST_F(NativeTypes, ConvertIntToJsValue)
{
    mofa::ref<jsValue> val = scriptingService->Execute("96;").As<mofa::ref<jsValue>>();
    EXPECT_EQ(96, fromJScript(val, jsType<int>()));
}

TEST_F(NativeTypes, NumberTests)
{
    EXPECT_EQ(-2, scriptingService->Execute("-Array(1,2).length;").As<int>());
    //EXPECT_DOUBLE_EQ(12345678*12345678., scriptingService->Execute("12345678*12345678;").As<int>());
}

TEST_F(NativeTypes, jsMathBinding)
{
    EXPECT_EQ(100, scriptingService->Execute("Math.pow(10,2);").As<int>());
    scriptingService->Execute("var ta = 0.1m/0.1m;");
    scriptingService->Execute("var nu = 0.7;");
    scriptingService->Execute("var tb = 0.1m/0.1m;");
    scriptingService->Execute("var k1 = Math.pow(ta, 2);");
    scriptingService->Execute("var k2 = Math.pow(tb, 2);");
    scriptingService->Execute("var q10 = 0.0;");
    scriptingService->Execute("var A = 3.0 * (3.0 - nu * nu)*(k1*k1 + k2 * k2) + 12.0*nu*k1*k2;");
    scriptingService->Execute("var Fy = 1MPa;");
    scriptingService->Execute("var E = 1MPa;");
    EXPECT_THROW(scriptingService->Execute("sin(2);"), std::exception);
    EXPECT_EQ(scriptingService->Execute("E;").As<Runtime::DynamicQuantity>(), 1_MPa);
    scriptingService->Execute("q10 = 0.1 * Math.pow(1,2)* Fy/E;");
    scriptingService->Execute("Math.pow(q10, 2);");

}

class One : public jsScriptable
{
public:
    One(const jsAutomation&) : value(0) {}
    One(double value) : value(value) {}
    static void init(jsTypeLibrary& typeLibrary)
    {
        jsClass<One> cls(typeLibrary, "One");
        cls.Constructor<double>();
        cls.Get("IsValueZero", &One::GetIsValueZero);
    }
    bool GetIsValueZero() const { return value == 0.0; }
    double value;
};

class Two : public jsScriptable
{
public:
    Two(const jsAutomation&) {}
    Two() {}
    static void init(jsTypeLibrary& typeLibrary)
    {
        jsClass<Two> cls(typeLibrary, "Two");
        cls.Constructor();
        cls.Get("one", &Two::GetOne);
        cls.Get("text", &Two::GetText);
    }
    One* GetOne() const { return new One(1034.3); }
    std::string GetText() { return "Hello"; }
};

TEST_F(NativeTypes, NestedJsClassCalling)
{
    One::init(jsStack::stack()->GetJsTypeLibrary());
    Two::init(jsStack::stack()->GetJsTypeLibrary());
    EXPECT_DOUBLE_EQ(1034.3, scriptingService->Execute("Two().one;").As<mofa::ref<One>>()->value);
    EXPECT_EQ("He", scriptingService->Execute("Two().text.slice(0,2);").As<std::string>());
}

TEST_F(NativeTypes, NativeTypesToString)
{
    DNVS::MoFa::Formatting::FormattingService formattingService;
    EXPECT_EQ("4.5", ToString(scriptingService->Execute("4.5;"), formattingService));
    EXPECT_EQ("4", ToString(scriptingService->Execute("4.0;"), formattingService));
    EXPECT_EQ("4.123456789", ToString(scriptingService->Execute("4.1234567891;"), formattingService));
    EXPECT_EQ("41234.56789", ToString(scriptingService->Execute("41234.56789123;"), formattingService));
    EXPECT_EQ("4.0", scriptingService->Execute("4.0.toString();").As<std::string>());
//     EXPECT_EQ("4.123456789123", ToString(scriptingService->Execute("4.1234567891;"), formattingService));
//     EXPECT_EQ("4.123456789123e4", ToString(scriptingService->Execute("41234.56789123;"), formattingService));
}

TEST_F(NativeTypes, MultiplyDoubleAsStringAndInt)
{
    DNVS::MoFa::Formatting::FormattingService formattingService;
    EXPECT_EQ(0.012, scriptingService->Execute("'0.012';").As<double>());
    EXPECT_EQ("12", ToString(scriptingService->Execute("'0.012' * 1000;"), formattingService));
    EXPECT_EQ("12", ToString(scriptingService->Execute("1000 * '0.012';"), formattingService));
}

TEST_F(NativeTypes, DivideDoubleAsStringAndInt)
{
    DNVS::MoFa::Formatting::FormattingService formattingService;
    EXPECT_EQ("1.2e-05", ToString(scriptingService->Execute("'0.012' / 1000;"), formattingService));
    EXPECT_EQ("83333.33333", ToString(scriptingService->Execute("1000 / '0.012';"), formattingService));
}

TEST_F(NativeTypes, ForLoop)
{
    scriptingService->Execute(
        "var a = 0;\n"
        "for(i = 0; i <= 2; i++)\n"
        "{\n"
        "   a = a + 1;\n"
        "   if(a>10) HelloWorld();\n"
        "}");
    scriptingService->Execute(
        "a = 0;\n"
        "for(i = 0; i <= 2; ++i)\n"
        "{\n"
        "   a = a + 1;\n"
        "   if(a>10) HelloWorld();\n"
        "}");
}

class ReflectedClass {
public:
    ReflectedClass(): m_value(0) {}
    void SetValue(int value) { m_value = value; }
    int GetValue() const { return m_value; }
private:
    int m_value;
};

static void DoReflect(TypeLibraries::TypeLibraryPointer typeLibrary, ReflectedClass**)
{
    using namespace Classes;
    Class<ReflectedClass> cls(typeLibrary, "ReflectedClass");
    cls.SetGet("Value", &ReflectedClass::SetValue, &ReflectedClass::GetValue);
}

class ClassWithCustomLookup : public jsScriptable
{
public:
    ClassWithCustomLookup() {}
    ClassWithCustomLookup(const jsAutomation&) {}
    static void init(jsTypeLibrary& typeLibrary)
    {
        jsClass<ClassWithCustomLookup> cls(typeLibrary, "ClassWithCustomLookup");
        cls.Constructor();        
    }
    virtual jsValue* lookup(const std::string& identifier, jsValue* owner = NULL)
    {
        auto object = Objects::Object::Create<ReflectedClass&>(ServiceProvider::Instance().GetService<TypeLibraries::ITypeLibrary>(), reflectedValue);
        if (object.HasMember(identifier))
        {
            auto wrapperService = ServiceProvider::Instance().TryGetService<IScriptingPropertyService>();
            return wrapperService->CreateDelegate(object, identifier);
        }
        return nullptr;
    }
    ReflectedClass reflectedValue;
};

TEST_F(NativeTypes, CustomLookup)
{
    ClassWithCustomLookup::init(jsStack::stack()->GetJsTypeLibrary());
    Reflect<ReflectedClass>(jsStack::stack()->GetTypeLibrary());
    scriptingService->Execute("a = ClassWithCustomLookup();");
    scriptingService->Execute("a.Value = 55;");
    const ClassWithCustomLookup& a = scriptingService->Execute("a;").As<const ClassWithCustomLookup&>();
    EXPECT_EQ(55, a.reflectedValue.GetValue());
    EXPECT_EQ(55, scriptingService->Execute("a.Value;").As<int>());
}

TEST_F(NativeTypes, TypeofDouble)
{
    ClassWithCustomLookup::init(jsStack::stack()->GetJsTypeLibrary());
    Reflect<ReflectedClass>(jsStack::stack()->GetTypeLibrary());
    EXPECT_EQ("double", scriptingService->Execute("typeof(55.4);").As<std::string>());
}


TEST_F(NativeTypes, FormatWithPrecision)
{
//    EXPECT_EQ("1.23456789", scriptingService->Execute("1.23456789;").As<std::string>());
    EXPECT_EQ("1.23456789", scriptingService->Execute("1.23456789.toString();").As<std::string>());
    EXPECT_EQ("1.2", scriptingService->Execute("1.2.toString();").As<std::string>());
    //     EXPECT_EQ("1.23456789", scriptingService->Execute("var a = 1.23456789;a;").As<std::string>());
    EXPECT_EQ("1.23456789", scriptingService->Execute("var b = 1.23456789;b.toString();").As<std::string>());
    EXPECT_EQ("1.23456789", scriptingService->Execute("1.23456789m.toDouble().toString();").As<std::string>());
}

TEST_F(NativeTypes, DefineVarTwice)
{    
    scriptingService->Execute("var a = 3;");
    EXPECT_NO_THROW(scriptingService->Execute("var a = 3;"));
}

TEST_F(NativeTypes, ValidateDoubleEqualsInt)
{
    EXPECT_EQ(1,scriptingService->Execute("2*0.5;").As<double>());
    EXPECT_TRUE(scriptingService->Execute("2*0.5 == 1;").As<bool>());
}

TEST_F(NativeTypes, ValidateCreateEmptyArray)
{
    EXPECT_NO_THROW(scriptingService->Execute("var fisk = new Array();"));
}

TEST_F(NativeTypes, RedefineVariable)
{
    scriptingService->Execute("function CreateLoad(lc) { return 1; }");
    EXPECT_NO_THROW(scriptingService->Execute("function CreateLoad(lc) { return 1; }"));
}

TEST_F(NativeTypes, DivideIntegers)
{
    EXPECT_DOUBLE_EQ(0.5,scriptingService->Execute("1/2;").As<double>());
}


TEST_F(NativeTypes, StringToIntTest)
{
    scriptingService->Execute(R"(name = "Jt2";)");
    EXPECT_EQ(2, scriptingService->Execute("name.substring(2,100);").As<int>());
    EXPECT_EQ(2, scriptingService->Execute("name.substring(2,100);").As<unsigned int>());
    EXPECT_EQ(2, scriptingService->Execute("name.substring(2,100);").As<long>());
    EXPECT_EQ(2, scriptingService->Execute("name.substring(2,100);").As<unsigned long>());
    EXPECT_EQ(2, scriptingService->Execute("name.substring(2,100);").As<short>());
    EXPECT_EQ(2, scriptingService->Execute("name.substring(2,100);").As<unsigned short>());
    EXPECT_EQ(2, scriptingService->Execute("name.substring(2,100);").As<std::int64_t>());
    EXPECT_EQ(2, scriptingService->Execute("name.substring(2,100);").As<std::uint64_t>());
}

TEST_F(NativeTypes, CompareUnits)
{
    scriptingService->Execute(R"(
z = Length(0);
y = Length(0);
Zlocal = z * Math.cos(20) - y * Math.sin(20);
Draught = 4.500000 m;
)");
    EXPECT_TRUE(scriptingService->Execute("Zlocal < Draught;").As<bool>());
    EXPECT_TRUE(scriptingService->Execute("Draught > Zlocal;").As<bool>());
}

TEST_F(NativeTypes, AppendUnits)
{
    scriptingService->Execute(R"(
z = Length(0);
y = Length(0);
Zlocal = z * Math.cos(20) - y * Math.sin(20);
Draught = 4.500000 m;
)");
    EXPECT_DOUBLE_EQ(0, scriptingService->Execute("Zlocal;").As<jsLength>());
    EXPECT_DOUBLE_EQ(4.5, scriptingService->Execute("Zlocal + Draught;").As<jsLength>());
    EXPECT_DOUBLE_EQ(4.5, scriptingService->Execute("Zlocal += Draught;").As<jsLength>());
    EXPECT_DOUBLE_EQ(4.5, scriptingService->Execute("Zlocal;").As<jsLength>());
}


TEST_F(NativeTypes, NegateUnit)
{
    scriptingService->Execute(R"(
z = Length(0);
y = Length(0);
Zlocal = z * Math.cos(20) - y * Math.sin(20);

)");
    EXPECT_DOUBLE_EQ(0, scriptingService->Execute("-Zlocal;").As<jsLength>());
}
using namespace DNVS::MoFa::Reflection::Types;
using namespace DNVS::MoFa::Reflection::TypeConversions;
template<typename MoFaClass, typename ScriptClass>
class jmfMoFaWrapperConversion : public IConversion
{

public:
    virtual DNVS::MoFa::Reflection::Variants::Variant Convert(const DNVS::MoFa::Reflection::Variants::Variant& variable) override
    {
        const MoFaClass& mofaClass = DNVS::MoFa::Reflection::Variants::InternalVariantService::UnreflectUnchecked<const MoFaClass&>(variable);
        mofa::ref<ScriptClass> scriptClass = new ScriptClass(mofaClass);
        return DNVS::MoFa::Reflection::Variants::VariantService::ReflectType<mofa::ref<ScriptClass>>(scriptClass);
    }


    virtual void IntrusiveConvert(DNVS::MoFa::Reflection::Variants::Variant& variable) override
    {
        variable = Convert(variable);
    }
};

class NativeClass {
public:
};

void DoReflect(TypeLibraries::TypeLibraryPointer typeLibrary, NativeClass**)
{
    using namespace DNVS::MoFa::Reflection::Classes;
    Class<NativeClass> cls(typeLibrary, "NativeClass");
    cls.Constructor();
}


class jsNativeClass : public jsScriptable
{
public:
    jsNativeClass(const jsAutomation& a) : jsScriptable(a) {}
    jsNativeClass(const NativeClass& native) 
    {
        m_inDummyMode = jsStack::stack()->dummyMode();
    }
    static NativeClass CreateNativeClass() { return NativeClass(); }
    bool InDummyMode() const { return m_inDummyMode; }

    static void Init(jsTypeLibrary& typeLibrary)
    {
        jsClass<jsNativeClass> cls(typeLibrary, "NativeClass");
        Reflect<NativeClass>(typeLibrary.GetReflectionTypeLibrary());
        typeLibrary.GetReflectionTypeLibrary()->GetConversionGraph()->AddConversion(
            TypeId<const NativeClass&>(),
            TypeId<mofa::ref<jsNativeClass>>(),
            ConversionType::UserConversion,
            std::make_shared<jmfMoFaWrapperConversion<const NativeClass&, jsNativeClass>>());
    }
private:
    bool m_inDummyMode;
};

TEST_F(NativeTypes, TestDummyMode)
{
    jsNativeClass::Init(jsStack::stack()->GetJsTypeLibrary());
    mofa::ref<jsNativeClass> native = scriptingService->Test("NativeClass();").As<mofa::ref<jsNativeClass>>();
    EXPECT_TRUE(native->InDummyMode());
    native = scriptingService->Execute("NativeClass();").As<mofa::ref<jsNativeClass>>();
    EXPECT_FALSE(native->InDummyMode());
}


class jsClassWithTwoSimilarOverloads : public jsScriptable
{
public:
    jsClassWithTwoSimilarOverloads() {}
    jsClassWithTwoSimilarOverloads(const jsAutomation& a) : jsScriptable(a) {}
    std::string FunctionWithString(const std::string& str) { return str; }
    int FunctionWithInt(int val) { return val+1; }
    static void Init(jsTypeLibrary& typeLibrary)
    {
        jsClass<jsClassWithTwoSimilarOverloads> cls(typeLibrary, "ClassWithTwoSimilarOverloads");
        cls.Constructor();
        cls.Function("Function", &jsClassWithTwoSimilarOverloads::FunctionWithString);
        cls.Function("Function", &jsClassWithTwoSimilarOverloads::FunctionWithInt);
    }
};

TEST_F(NativeTypes, TestClassWithTwoSimilarOverloads)
{
    jsClassWithTwoSimilarOverloads::Init(jsStack::stack()->GetJsTypeLibrary());
    scriptingService->Execute("A = ClassWithTwoSimilarOverloads();");
    EXPECT_EQ("Hello", scriptingService->Execute(R"( A.Function("Hello"); )").As<std::string>());
    EXPECT_EQ(56, scriptingService->Execute(R"( A.Function(55); )").As<int>());
    EXPECT_EQ(1, scriptingService->Execute(R"( A.Function(0); )").As<int>());
}

TEST_F(NativeTypes, PopItemsFromBack)
{
   scriptingService->Execute(R"(
A = Array();
A.push(5);
A.push(6);
A.push(7);
    )");
    scriptingService->Execute("for (i = A.length - 1; i >= 0; i--) A.pop();");
}

TEST_F(NativeTypes, TestTypeof0)
{
    scriptingService->Execute("var a = 0;");
    std::string type = scriptingService->Execute("typeof(a);").As<std::string>();
    std::set<std::string> validValues = { "long", "integer" };
    EXPECT_TRUE(validValues.count(type) > 0);
}

TEST_F(NativeTypes, TestCompareLongWithNull)
{
    scriptingService->Execute("a = 1;");
    EXPECT_FALSE(scriptingService->Execute("a == null;").As<bool>());
    EXPECT_TRUE(scriptingService->Execute("a != null;").As<bool>());
}

class jsClassThatReturnsNull : public jsScriptable
{
public:
    jsClassThatReturnsNull() {}
    jsClassThatReturnsNull(const jsAutomation& a) : jsScriptable(a) {}
    jsValue* ReturnNull() { return nullptr; }
    static void Init(jsTypeLibrary& typeLibrary)
    {
        jsClass<jsClassThatReturnsNull> cls(typeLibrary, "ClassThatReturnsNull");
        cls.Constructor();
        cls.Function("ReturnNull", &jsClassThatReturnsNull::ReturnNull);
        cls.Get("ReturnNullPr", &jsClassThatReturnsNull::ReturnNull);
    }
};


TEST_F(NativeTypes, TestClassThatReturnsNull)
{
    jsClassThatReturnsNull::Init(jsStack::stack()->GetJsTypeLibrary());
    scriptingService->Execute("A = ClassThatReturnsNull();");
    EXPECT_TRUE(scriptingService->Execute("A.ReturnNull() == null;").As<bool>());
    EXPECT_TRUE(scriptingService->Execute("A.ReturnNullPr == null;").As<bool>());
}

TEST_F(NativeTypes, ShiftRightOnLong)
{
    scriptingService->Execute("A = 5;");
    EXPECT_TRUE(scriptingService->Execute("A >> 0;").As<bool>());
    scriptingService->Execute("A = 0;");
    EXPECT_FALSE(scriptingService->Execute("A >> 0;").As<bool>());
}

TEST_F(NativeTypes, AssignOutOfRangeInArrayShouldWork)
{
    scriptingService->Execute("var A = new Array();");
    EXPECT_NO_THROW(scriptingService->Execute("A.length;"));
    EXPECT_NO_THROW(scriptingService->Execute("A[A.length] = 4;"));
}

TEST_F(NativeTypes, AddStringAndUndefined)
{
    scriptingService->Execute("var kn;");
    EXPECT_EQ("kn = undefined", scriptingService->Execute("'kn = ' + kn;").As<std::string>());
    EXPECT_EQ("undefinedkn = ", scriptingService->Execute("kn + 'kn = ';").As<std::string>());
}

TEST_F(NativeTypes, AddStringAndNull)
{
    EXPECT_EQ("kn = null", scriptingService->Execute("'kn = ' + null;").As<std::string>());
    EXPECT_EQ("null = kn", scriptingService->Execute("null + ' = kn';").As<std::string>());
}

TEST_F(NativeTypes, BitwiseAndBoolean)
{
    EXPECT_EQ(true, scriptingService->Execute("true&true;").As<bool>());
    EXPECT_EQ(true, scriptingService->Execute("'A' == 'A' & 'B' =='B';").As<bool>());
}

TEST_F(NativeTypes, CompareStringWithNull)
{
    EXPECT_EQ(true, scriptingService->Execute("null!='Hello';").As<bool>());
    EXPECT_EQ(true, scriptingService->Execute("'Hello'!=null;").As<bool>());
    EXPECT_EQ(false, scriptingService->Execute("'null'==null;").As<bool>());
    EXPECT_EQ(false, scriptingService->Execute("null=='null';").As<bool>());
}

TEST_F(NativeTypes, DivideUnitWithJsUnit)
{
    scriptingService->Execute(R"(
    var a = Length(31);
    var b = Length(41);
    var t = Length(11);
    var ta = t / a;
    var tb = t / b;
    var k1 = Math.pow(ta, 2);
    var k2 = Math.pow(tb, 2);
    var E = ForcePerUnitArea(13454323);
    var nu = 0.7;
    var A = 3.0 * ( 3.0 - nu*nu )*( k1*k1 + k2*k2 ) + 12.0*nu*k1*k2;
    )");
    EXPECT_NO_THROW(scriptingService->Execute("var C11 = (E / (1 - nu * nu))      * (1.0 - (6.0          * Math.pow((k1 + nu * k2), 2) / A));"));
    EXPECT_NO_THROW(scriptingService->Execute("var C12 = (E * nu / (1 - nu * nu)) * (1.0 - ((6.0 / nu) * ((k1 + nu * k2) * (k2 + nu * k1) / A)));"));
    EXPECT_NO_THROW(scriptingService->Execute("var C22 = (E / (1 - nu * nu))      * (1.0 - (6.0          * Math.pow((k2 + nu * k1), 2) / A));"));
    EXPECT_NO_THROW(scriptingService->Execute("var C33 = (E / (2 * (1 + nu)));"));
    scriptingService->Execute("var nu12 = C12 / C22;");
    EXPECT_NO_THROW(EXPECT_DOUBLE_EQ(-0.064503482893590577, scriptingService->Execute("nu12;").As<double>()));
}

TEST_F(NativeTypes, ReuseVariableFromReflectionToJs)
{
    scriptingService->Execute(R"(
        var a = 0.01 m;
       	a = Length(3);
    )");
}

TEST_F(NativeTypes, ForLoopPerformance)
{
    std::chrono::time_point<std::chrono::system_clock> startTime = std::chrono::system_clock::now();
    {
        scriptingService->Execute(R"(
        var i=0;
        do 
        {
          i++;
        }
        while (i < 1000000);
        )");
        EXPECT_EQ(1000000, scriptingService->Execute("i;").As<int>());
    }
    std::chrono::time_point<std::chrono::system_clock> endTime = std::chrono::system_clock::now();
    EXPECT_GT(10, std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count());
}

TEST_F(NativeTypes, DoubleToInt)
{
    EXPECT_EQ(1, scriptingService->Execute(R"(
        var a = 1.2;
       	a.toInt();
    )").As<double>());
}

TEST_F(NativeTypes, IntToDouble)
{
    EXPECT_EQ(1, scriptingService->Execute(R"(
        var a = 1;
       	a.toFloat();
    )").As<double>());
}

TEST_F(NativeTypes, IntToInt)
{
    EXPECT_EQ(1, scriptingService->Execute(R"(
        var a = 1;
       	a.toInt();
    )").As<double>());
}

TEST_F(NativeTypes, DoubleToDouble)
{
    EXPECT_EQ(1.2, scriptingService->Execute(R"(
        var a = 1.2;
       	a.toFloat();
    )").As<double>());
}

bool GetBool(bool value) { return value; }
TEST_F(NativeTypes, BoolToInt)
{
    using namespace DNVS::MoFa::Reflection::Classes;
    using namespace DNVS::MoFa::Reflection::Members;
    Class<GlobalType>(scriptingService->GetTypeLibrary(), "").StaticFunction("GetBool", &GetBool);
    EXPECT_EQ(1, scriptingService->Execute(R"(
        var a = GetBool(true);
       	a.toInt();
    )").As<int>());
}

TEST_F(NativeTypes, BoolToDouble)
{
    using namespace DNVS::MoFa::Reflection::Classes;
    using namespace DNVS::MoFa::Reflection::Members;
    Class<GlobalType>(scriptingService->GetTypeLibrary(), "").StaticFunction("GetBool", &GetBool);
    EXPECT_EQ(0, scriptingService->Execute(R"(
        var a = GetBool(false);
       	a.toFloat();
    )").As<double>());
}

TEST_F(NativeTypes, ToFixed)
{
    EXPECT_EQ("123456789", scriptingService->Execute(R"(
        var a = 1.2345678912345e8;
       	a.toFixed();
    )").As<std::string>());
    EXPECT_EQ("123456789.12345", scriptingService->Execute(R"(
        var a = 1.2345678912345e8;
       	a.toFixed(5);
    )").As<std::string>());
    EXPECT_EQ("1", scriptingService->Execute(R"(
        var a = 1.2345678912345;
       	a.toFixed();
    )").As<std::string>());
    EXPECT_THROW(scriptingService->Execute(R"(
        var a = 1123;
       	a.toFixed(1);
    )"), std::exception);
    EXPECT_EQ("1234567891235", scriptingService->Execute(R"(
        var a = 1.2345678912345e12;
       	a.toFixed();
    )").As<std::string>());
}

TEST_F(NativeTypes, ToExponential)
{
    EXPECT_EQ("1.234568e+08", scriptingService->Execute(R"(
        var a = 1.2345678912345e8;
       	a.toExponential();
    )").As<std::string>());
    EXPECT_EQ("1.23457e+08", scriptingService->Execute(R"(
        var a = 1.2345678912345e8;
       	a.toExponential(5);
    )").As<std::string>());
    EXPECT_EQ("1e+00", scriptingService->Execute(R"(
        var a = 1.2345678912345;
       	a.toExponential(0);
    )").As<std::string>());
    EXPECT_THROW(scriptingService->Execute(R"(
        var a = 1123;
       	a.toExponential(1);
    )"), std::exception);
    EXPECT_EQ("1.23456789123e+12", scriptingService->Execute(R"(
        var a = 1.2345678912345e12;
       	a.toExponential(11);
    )").As<std::string>());
}

TEST_F(NativeTypes, ToPrecision)
{
    EXPECT_EQ("1.2345678912e08", scriptingService->Execute(R"(
        var a = 1.2345678912345e8;
       	a.toPrecision();
    )").As<std::string>());
    EXPECT_EQ("1.23457e08", scriptingService->Execute(R"(
        var a = 1.2345678912345e8;
       	a.toPrecision(5);
    )").As<std::string>());
    EXPECT_EQ("123.4", scriptingService->Execute(R"(
        var a = 123.4;
       	a.toPrecision();
    )").As<std::string>());
    EXPECT_EQ("123.4", scriptingService->Execute(R"(
        var a = 123.4;
       	a.toPrecision(5);
    )").As<std::string>());
    EXPECT_EQ("1.", scriptingService->Execute(R"(
        var a = 1.2345678912345;
       	a.toPrecision(0);
    )").As<std::string>());
    EXPECT_THROW(scriptingService->Execute(R"(
        var a = 1123;
       	a.toPrecision(1);
    )"), std::exception);
    EXPECT_EQ("1.23456789123e12", scriptingService->Execute(R"(
        var a = 1.2345678912345e12;
       	a.toPrecision(11);
    )").As<std::string>());
}

TEST_F(NativeTypes, StaticMembersInClassTests)
{
    EXPECT_NO_THROW(scriptingService->Execute(R"(
        class cStdDetailTypes {
            static const typeNone = 0;
            static const typeTransFrameSingle = 101;
            static const typeTransFrameMulti = 102;
            static const typeTransBulkhead = 103;
        };
    )"));
    scriptingService->Execute("var a = new cStdDetailTypes();");
    EXPECT_EQ(0, scriptingService->Execute("a.typeNone;").As<int>());
    EXPECT_EQ(101, scriptingService->Execute("a.typeTransFrameSingle;").As<int>());
    EXPECT_THROW(scriptingService->Execute("a.typeNone = 5;"), std::exception);
    EXPECT_THROW(scriptingService->Execute("a.typeNone += 5;"), std::exception);
    EXPECT_THROW(scriptingService->Execute("a.typeNone--;"), std::exception);
    EXPECT_THROW(scriptingService->Execute("++a.typeNone;"), std::exception);
    EXPECT_EQ(0, scriptingService->Execute("a.typeNone;").As<int>());
    EXPECT_EQ(103, scriptingService->Execute("cStdDetailTypes.typeTransBulkhead;").As<int>());
}

TEST_F(NativeTypes, ConstructorAndFunctionsInCustomClass)
{
    EXPECT_NO_THROW(scriptingService->Execute(R"(
        class ctest {
            var prefix;
            constructor function ctest(myString) {
                prefix = myString;
            }
            function PrintColParams() {
                return "prefix : " + prefix.toString();
            }
        };
    )"));
    scriptingService->Execute(R"(mytest = new ctest("my obj prefix");)");
    EXPECT_EQ("prefix : my obj prefix", scriptingService->Execute("mytest.PrintColParams();").As<std::string>());
}

TEST_F(NativeTypes, ArrayReuseIndex)
{
    scriptingService->Execute("var a = new Array();");
    scriptingService->Execute("a[0] = 4;");
    scriptingService->Execute("a[0] = 5;");
    EXPECT_EQ(5, scriptingService->Execute("a[0];").As<int>());
}


TEST_F(NativeTypes, ArrayReuseIndexJScriptClass)
{
    One::init(jsStack::stack()->GetJsTypeLibrary());
    scriptingService->Execute("var a = new Array();");
    scriptingService->Execute("a[0] = One(4);");
    scriptingService->Execute("a[0] = One(5);");
    EXPECT_EQ(5, scriptingService->Execute("a[0];").As<One*>()->value);
}

TEST_F(NativeTypes, JoinStringWithNumber)
{
    EXPECT_EQ("Hello", scriptingService->Execute("'Hello';").As<std::string>());
    EXPECT_EQ("1", scriptingService->Execute("'1';").As<std::string>());
    EXPECT_EQ("11", scriptingService->Execute("'1' + 1;").As<std::string>());
    EXPECT_EQ("112", scriptingService->Execute("'1' + 1 + 2;").As<std::string>());
    EXPECT_EQ(3, scriptingService->Execute("1 + 2;").As<int>());
    EXPECT_EQ("3 3", scriptingService->Execute("1 + 2 + ' 3';").As<std::string>());
    EXPECT_EQ("33", scriptingService->Execute("1 + 2 + '3';").As<std::string>());
    EXPECT_EQ("1.23", scriptingService->Execute("1.2 + '3';").As<std::string>());
    EXPECT_EQ("1.23.2", scriptingService->Execute("'1.2' + 3.2;").As<std::string>());
    EXPECT_EQ("1.23.0", scriptingService->Execute("'1.2' + 3.0;").As<std::string>());
    EXPECT_EQ("1.23.0", scriptingService->Execute("'1.2' + 3.0000;").As<std::string>());
    EXPECT_EQ("1.23.1234567891", scriptingService->Execute("'1.2' + 3.123456789123;").As<std::string>());
    EXPECT_EQ("1.23.1234567891e06", scriptingService->Execute("'1.2' + 3.123456789123e6;").As<std::string>());
    EXPECT_EQ("1.231234.56789123", scriptingService->Execute("'1.2' + 3.123456789123e4;").As<std::string>());
}

TEST_F(NativeTypes, ConditionalExpressionOr)
{
    scriptingService->Execute("buildLower = !false || !skipLower;");
    EXPECT_EQ(true, scriptingService->Execute("buildLower;").As<bool>());
    EXPECT_EQ(GetBoolTypeString(), scriptingService->Execute("typeof(buildLower);").As<std::string>());
    EXPECT_NO_THROW(scriptingService->Execute("if(buildLower);"));
}

TEST_F(NativeTypes, ConditionalExpressionJsProperty)
{
    //This is not supported for old scripting engine
    One::init(jsStack::stack()->GetJsTypeLibrary());
    EXPECT_TRUE(scriptingService->Execute("One(1).IsValueZero || true;").As<bool>());
    EXPECT_FALSE(scriptingService->Execute("false || One(1).IsValueZero;").As<bool>());
    EXPECT_FALSE(scriptingService->Execute("true && One(1).IsValueZero;").As<bool>());
}

TEST_F(NativeTypes, ConditionalExpressionAnd)
{
    scriptingService->Execute("buildLower = !false && false;");
    EXPECT_EQ(false, scriptingService->Execute("buildLower;").As<bool>());
    EXPECT_EQ(GetBoolTypeString(), scriptingService->Execute("typeof(buildLower);").As<std::string>());
    EXPECT_NO_THROW(scriptingService->Execute("if(buildLower);"));
}

TEST_F(NativeTypes, TestArrays)
{
    scriptingService->Execute(R"(
    function PIPE_INIT() {

        PipeArray = new Array();

        PipeArray[0] = new Array();

        PipeArray[1] = new Array();

        PipeArray[2] = new Array();

        PipeArray[3] = new Array();

        return PipeArray;

    }
    )");
    scriptingService->Execute("PipeArray = PIPE_INIT();");
}
namespace ixion { namespace javascript { class integer; } }
namespace ixion { namespace javascript { class nonExistingType; } }
TEST_F(NativeTypes, TestType_HideIxion)
{
    EXPECT_EQ("integer", jsStack::stack()->typeName(typeid(ixion::javascript::integer)));
    EXPECT_EQ("integer *", scriptingService->GetTypeLibrary()->GetTypeFormatter()->FormatType(Types::TypeId<ixion::javascript::integer*>()));
    EXPECT_EQ("nonExistingType *", scriptingService->GetTypeLibrary()->GetTypeFormatter()->FormatType(Types::TypeId<ixion::javascript::nonExistingType*>()));
}

TEST_F(NativeTypes, GetCharIndexTests)
{
    scriptingService->Execute(R"(
    function getCharIndex(str, token) {
        for (var i = 0; i < str.length; i++) {
            if (str.substring(i, i+1) == token) {
                return i;
            }
        }
    }
)");
    EXPECT_EQ(1, scriptingService->Execute("getCharIndex('test','e');").As<int>());
    EXPECT_EQ(4, scriptingService->Execute("getCharIndex('test test',' ');").As<int>());
    EXPECT_EQ(0, scriptingService->Execute("getCharIndex('test test','t');").As<int>());
    EXPECT_EQ(4, scriptingService->Execute("'test test'.indexOf(' ');").As<int>());
    EXPECT_EQ(7, scriptingService->Execute("'Hello word'.indexOf('or');").As<int>());
}

TEST_F(NativeTypes, PushMultipleItemsToArray)
{
    scriptingService->Execute("myArray = new Array();");
    EXPECT_NO_THROW(scriptingService->Execute("myArray.push(4);"));
    EXPECT_NO_THROW(scriptingService->Execute("myArray.push(5,6);"));
    EXPECT_EQ(3,scriptingService->Execute("myArray.length;").As<int>());
}

TEST_F(NativeTypes, CompareNullAndUndefined)
{
    EXPECT_TRUE(scriptingService->Execute("null == undefined;").As<bool>());
    EXPECT_FALSE(scriptingService->Execute("null != undefined;").As<bool>());
    EXPECT_TRUE(scriptingService->Execute("null == null;").As<bool>());
    EXPECT_FALSE(scriptingService->Execute("null != null;").As<bool>());
    EXPECT_FALSE(scriptingService->Execute("undefined != undefined;").As<bool>());
    EXPECT_TRUE(scriptingService->Execute("undefined == undefined;").As<bool>());

    EXPECT_FALSE(scriptingService->Execute("null === undefined;").As<bool>());
    EXPECT_TRUE(scriptingService->Execute("null !== undefined;").As<bool>());
    EXPECT_TRUE(scriptingService->Execute("null === null;").As<bool>());
    EXPECT_TRUE(scriptingService->Execute("undefined === undefined;").As<bool>());
    EXPECT_FALSE(scriptingService->Execute("null !== null;").As<bool>());
    EXPECT_FALSE(scriptingService->Execute("undefined !== undefined;").As<bool>());
}

TEST_F(NativeTypes, CallArrayAsMethod)
{
    scriptingService->Execute("var stiffenerArrAtCurElev = new Array();");
    EXPECT_THROW(scriptingService->Execute("stiffenerArrAtCurElev(1,2);"), std::exception);
    EXPECT_EQ(0, scriptingService->Execute("stiffenerArrAtCurElev.length;").As<int>());
}

TEST_F(NativeTypes, CallMethodWithTooFewArguments)
{
    scriptingService->Execute(R"(function MultiDimensionalArray(iRows, iCols) {
        var i;
        var j;
        var a = new Array(iRows);
        for (var i = 0; i < iRows; i++) {
            a[i] = new Array(iCols);
            for (var j = 0; j < iCols; j++) {
                a[i][j] = "";
            }
        }
        return(a);
    }
    function CreateCircleGuideCurvesAtOffset(pointDeltaArr){
          // setting initial values for optional parameter: pointDeltaArr
	    if (pointDeltaArr == undefined) {
		    pointDeltaArr = MultiDimensionalArray(16,2);
		    // looping over all 16 points
		    for (i=0;i<16;i++){ // looping over x and y cooridnate
			    for (j=0;j<3;j++){
				    pointDeltaArr[i][j] = 0;
			    }			
		    }
	    }
    })");
    EXPECT_NO_THROW(scriptingService->Execute("CreateCircleGuideCurvesAtOffset();"));
}

TEST_F(NativeTypes, AccessArrayElementInsideFunctionCall)
{
    scriptingService->Execute(R"(
    function AppendArray(targetArr, sourceArr) {
        for (var item in sourceArr) {
            targetArr.push(item);
        }
    })");
    scriptingService->Execute("var targetArray = Array();");
    scriptingService->Execute(R"(function CreateEmptySource() 
{
	var returnArr = new Array();
	var returnPlateArr = new Array();
	var returnBeamArr = new Array();
	returnArr.push(returnPlateArr);
	returnArr.push(returnBeamArr);
    return returnArr[0];
})");
    scriptingService->Execute("AppendArray(targetArray, CreateEmptySource());");
}

TEST_F(NativeTypes, AccessArrayElementOutsideFunctionCall)
{
    scriptingService->Execute(R"(
    function AppendArray(targetArr, sourceArr) {
        for (var item in sourceArr) {
            targetArr.push(item);
        }
    })");
    scriptingService->Execute("var targetArray = Array();");
    scriptingService->Execute(R"(function CreateEmptySource() 
{
	var returnArr = new Array();
	var returnPlateArr = new Array();
	var returnBeamArr = new Array();
	returnArr.push(returnPlateArr);
	returnArr.push(returnBeamArr);
    return returnArr;
})");
    scriptingService->Execute("AppendArray(targetArray, CreateEmptySource()[0]);");
}

TEST_F(NativeTypes, CompareNullWithNullReturnedFromFunction)
{
    scriptingService->Execute(R"(
    function returnNull()
    {
        var v = null;
        return v;
    }

    t = returnNull();
    )");
    ASSERT_NO_THROW(scriptingService->Execute("t == null;"));
    EXPECT_TRUE(scriptingService->Execute("t == null;").As<bool>());
    ASSERT_NO_THROW(scriptingService->Execute("t != null;"));
    ASSERT_NO_THROW(scriptingService->Execute("t < null;"));
    ASSERT_NO_THROW(scriptingService->Execute("!t;"));
    EXPECT_TRUE(scriptingService->Execute("!t;").As<bool>());
    EXPECT_FALSE(scriptingService->Execute("t > null;").As<bool>());
    EXPECT_TRUE(scriptingService->Execute("1 > t;").As<bool>());
    EXPECT_TRUE(scriptingService->Execute("t <= null;").As<bool>());
    EXPECT_TRUE(scriptingService->Execute("t >= null;").As<bool>());
    EXPECT_EQ(0,scriptingService->Execute("t + null;").As<int>());
    EXPECT_EQ(5, scriptingService->Execute("t + 5;").As<int>());
    EXPECT_EQ(5, scriptingService->Execute("5 + t;").As<int>());
    EXPECT_EQ(0, scriptingService->Execute("t - null;").As<int>());
    EXPECT_EQ(FP_NAN, fpclassify(scriptingService->Execute("t / null;").As<double>()));
    EXPECT_EQ(FP_NAN, fpclassify(scriptingService->Execute("t / 0;").As<double>()));
    EXPECT_EQ(FP_NAN, fpclassify(scriptingService->Execute("0 / t;").As<double>()));
    EXPECT_EQ(0, scriptingService->Execute("t * null;").As<int>());
    EXPECT_EQ(0, scriptingService->Execute("-t;").As<int>());
}

TEST_F(NativeTypes, CompareNullWithNull)
{
    scriptingService->Execute("t = null;");
    ASSERT_NO_THROW(scriptingService->Execute("t == null;"));
    EXPECT_TRUE(scriptingService->Execute("t == null;").As<bool>());
    ASSERT_NO_THROW(scriptingService->Execute("t != null;"));
    ASSERT_NO_THROW(scriptingService->Execute("t < null;"));
    ASSERT_NO_THROW(scriptingService->Execute("!t;"));
    EXPECT_TRUE(scriptingService->Execute("!t;").As<bool>());
    EXPECT_FALSE(scriptingService->Execute("t > null;").As<bool>());
    EXPECT_TRUE(scriptingService->Execute("1 > t;").As<bool>());
    EXPECT_TRUE(scriptingService->Execute("t <= null;").As<bool>());
    EXPECT_TRUE(scriptingService->Execute("t >= null;").As<bool>());
    EXPECT_EQ(0, scriptingService->Execute("t + null;").As<int>());
    EXPECT_EQ(5, scriptingService->Execute("t + 5;").As<int>());
    EXPECT_EQ(5, scriptingService->Execute("5 + t;").As<int>());
    EXPECT_EQ(0, scriptingService->Execute("t - null;").As<int>());
    EXPECT_EQ(FP_NAN, fpclassify(scriptingService->Execute("t / null;").As<double>()));
    EXPECT_EQ(FP_NAN, fpclassify(scriptingService->Execute("t / 0;").As<double>()));
    EXPECT_EQ(FP_NAN, fpclassify(scriptingService->Execute("0 / t;").As<double>()));
    EXPECT_EQ(0, scriptingService->Execute("t * null;").As<int>());
    EXPECT_EQ(0, scriptingService->Execute("-t;").As<int>());
}

TEST_F(NativeTypes, SubtractLengthsOfStrings)
{
    EXPECT_EQ(-2, scriptingService->Execute(R"("1234".length - "123456".length;)").As<int>());
    EXPECT_EQ("-2", scriptingService->Execute(R"("1234".length - "123456".length;)").As<std::string>());
}

TEST_F(NativeTypes, TestIncorrectUseOfPopOnArray)
{
    scriptingService->Execute(R"(
    a = Array(1,2,3);
   for (var i in a){
      myPrefixSets = new Array();
      myPrefixSets.push(i); 
   }
  //Dette tømmer myPrefixSets, slik at størrelsen blir 0
   while(myPrefixSets.length > 0) {
      myPrefixSets.pop();
   }
    )");
    scriptingService->Execute("myPrefixSets.pop();");
}

TEST_F(NativeTypes, TestSortOfArray)
{
    scriptingService->Execute(R"(
    a = Array("He","Fe","Ga","Gra");
    a.sort();
    )");
    EXPECT_EQ("Fe,Ga,Gra,He", scriptingService->Execute("a.join();").As<std::string>());
}

TEST_F(NativeTypes, TestCustomSortOfArray)
{
    scriptingService->Execute(R"(
    a = Array(1,11,2);
    a.sort(function(a,b) {return a-b;});
    )");
    EXPECT_EQ("1,2,11", scriptingService->Execute("a.join();").As<std::string>());
}

TEST_F(NativeTypes, TestModulusDouble)
{
    EXPECT_DOUBLE_EQ(scriptingService->Execute("2.5%2;").As<double>(), 0.5);
    EXPECT_DOUBLE_EQ(scriptingService->Execute("1.5%2;").As<double>(), 1.5);
    EXPECT_DOUBLE_EQ(scriptingService->Execute("2.5%2.1;").As<double>(), 0.3999999999999999);
    EXPECT_DOUBLE_EQ(scriptingService->Execute("-2.5%2.1;").As<double>(), -0.3999999999999999);
    EXPECT_DOUBLE_EQ(scriptingService->Execute("2.5%-2;").As<double>(), 0.5);
    EXPECT_DOUBLE_EQ(scriptingService->Execute("3%2.5;").As<double>(), 0.5);
    EXPECT_DOUBLE_EQ(scriptingService->Execute("3.0%2.5;").As<double>(), 0.5);
    EXPECT_DOUBLE_EQ(scriptingService->Execute("isNaN(2%NAN);").As<bool>(), true);
    EXPECT_DOUBLE_EQ(scriptingService->Execute("isNaN(2.0%NAN);").As<bool>(), true);
    EXPECT_DOUBLE_EQ(scriptingService->Execute("isNaN(NAN%2);").As<bool>(), true);
    EXPECT_DOUBLE_EQ(scriptingService->Execute("isNaN(NAN%2.0);").As<bool>(), true);
    EXPECT_DOUBLE_EQ(scriptingService->Execute("isNaN(2.5%0);").As<bool>(), true);
    EXPECT_DOUBLE_EQ(scriptingService->Execute("2%0.5;").As<double>(), 0);
    EXPECT_DOUBLE_EQ(scriptingService->Execute("0%2.5;").As<double>(), 0);
}
