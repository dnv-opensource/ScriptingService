#include "gtest\gtest.h"
#include <list>
#include "Scripting\IScriptingService.h"
#include "Scripting\Scripting.h"
#include "Reflection\Classes\Class.h"
#include "Services\ScopedServiceProvider.h"
#include <Reflection/Containers/ReflectList.h>
#include "jsScript\jsStack.h"
#include "ScriptingServiceFactory.h"
#include "jsScript\jsArray.h"
#include "jsScript\jsConversions.h"
#include "Reflection\Members\GlobalType.h"

using namespace DNVS::MoFa::Scripting;
using namespace DNVS::MoFa::Reflection;

class jsArrayTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_scriptingService = ScriptingServiceFactory().CreateScriptingService();
        m_sp.RegisterService(jsStack::stack()->GetTypeLibrary());
        m_sp.RegisterService(m_scriptingService);
    }

    DNVS::MoFa::Services::ScopedServiceProvider m_sp;
    ScriptingServicePointer m_scriptingService;
};

class ClassWithList {
public:
    ClassWithList(const std::list<int>& args)
        : m_args(args) {}
    std::list<int> GetArgs() const { return m_args; }
    void SetArgs(const std::list<int>& args) { m_args = args; }
private:
    std::list<int> m_args;
};
typedef std::shared_ptr<ClassWithList> ClassWithListPointer;
void DoReflect(TypeLibraries::TypeLibraryPointer typeLibrary, ClassWithList**)
{
    using namespace Classes;
    Class<ClassWithList, ClassWithListPointer> cls(typeLibrary, "ClassWithList");
    cls.Constructor<const std::list<int>&>().AddSignature("args");
    cls.SetGet("Args", &ClassWithList::SetArgs, &ClassWithList::GetArgs);
}

TEST_F(jsArrayTests, CreateArray_ConvertToList)
{
    Reflect<ClassWithList>(m_scriptingService->GetTypeLibrary());
    ClassWithListPointer classWithList = Execute("ClassWithList(Array(1,2,5));").As<ClassWithListPointer>();
    EXPECT_EQ((std::list<int>{ 1,2,5 }), classWithList->GetArgs());
}

TEST_F(jsArrayTests, CreateOldStyleArray_AccessWithVectorSubscript)
{
    using namespace Classes;
    Class<Members::GlobalType> cls(m_scriptingService->GetTypeLibrary(), "");
    cls.StaticFunction("GetArray", []() { return new jsArray(jsValue::Params(1, toJScript(5))); });
    Execute("GetArray()[0];");
}

TEST_F(jsArrayTests, CreateArrayOfArray_Join)
{
    using namespace Classes;
    Execute(R"(
var a = new Array();
var b = new Array();
b.push(1);
a.push(b);
)");
    EXPECT_EQ("1", Execute("a.join(';');").As<std::string>());
}