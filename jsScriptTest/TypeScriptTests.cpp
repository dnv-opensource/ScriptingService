#include "ScriptingServiceFactory.h"
#include "gtest/gtest.h"
#include "GoogleTest/gtest_assert_at.h"
#include "Reflection/Members/IMember.h"

using namespace DNVS::MoFa::Reflection::Members;
using namespace DNVS::MoFa::Reflection::Types;
TEST(TypeScriptTests, ExecuteScriptContainingArgumentTypes)
{
    auto scriptingService = ScriptingServiceFactory().CreateScriptingService();
    EXPECT_NO_THROW_EXCEPTION(scriptingService->Execute("a = function(a : number, b : number) : number {return a+b;};"), std::exception);
}

TEST(TypeScriptTests, InvokeCustomFunction)
{
    auto scriptingService = ScriptingServiceFactory().CreateScriptingService();
    scriptingService->Execute("a = function(a : number, b : number) : number {return a+b;};");
    EXPECT_DOUBLE_EQ(scriptingService->Execute("a(5,6);").As<double>(), 11);
}

TEST(TypeScriptTests, ExecuteScriptContainingArgumentTypes_ConvertToMember)
{
    auto scriptingService = ScriptingServiceFactory().CreateScriptingService();
    MemberPointer member = scriptingService->Execute("a = function(a : number, b : number) : number {return a+b;};").As<MemberPointer>();
    EXPECT_EQ(member->GetArity(), 2);
    EXPECT_EQ(member->GetArgumentInfo(0)->GetTypeInfo(), TypeId<double>());
}

TEST(TypeScriptTests, AddAttributes)
{
    auto scriptingService = ScriptingServiceFactory().CreateScriptingService();
    scriptingService->Execute("MyFun = function(a : number, b : number) : number {return a+b;};");
    scriptingService->Execute("MyFun['a'].AddAttribute(CaptionAttribute('My Caption'));");
}
TEST(TypeScriptTests, FormatSignature)
{
    auto scriptingService = ScriptingServiceFactory().CreateScriptingService();
    MemberPointer member = scriptingService->Execute("a = function(a : number, b : number) : number {return a+b;};").As<MemberPointer>();
    EXPECT_EQ(member->Format(IMember::FormatType::FunctionSignature), std::string("function(a : number, b : number) : number"));
}

TEST(TypeScriptTests, FormatBody)
{
    auto scriptingService = ScriptingServiceFactory().CreateScriptingService();
    MemberPointer member = scriptingService->Execute("a = function(a : number, b : number) : number {return a+b;};").As<MemberPointer>();
    EXPECT_EQ(member->Format(IMember::FormatType::FunctionBody), std::string("return a + b;\r\n"));
}

TEST(TypeScriptTests, FormatFunction)
{
    auto scriptingService = ScriptingServiceFactory().CreateScriptingService();
    MemberPointer member = scriptingService->Execute("a = function(a : number, b : number) : number {return a+b;};").As<MemberPointer>();
    EXPECT_EQ(member->Format(IMember::FormatType::Function), std::string("function(a : number, b : number) : number\r\n{\r\nreturn a + b;\r\n}"));
}