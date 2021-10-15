#include <gtest/gtest.h>
#include "Scripting/ScriptExpressionEvaluator.h"
#include "Services/ScopedServiceProvider.h"
#include "Units/Length.h"
#include "ScriptingServiceFactory.h"

using namespace DNVS::MoFa::Scripting;
using namespace DNVS::MoFa::Reflection;
using namespace DNVS::MoFa::Services;
using namespace DNVS::MoFa::Units;


template<typename T>
void TestExpression(std::shared_ptr<IScriptingService> scriptingService, const std::string& text, size_t startIndex, size_t expectedStart, size_t expectedEnd, const T& expectedValue, bool isEnd)
{
    ScriptExpressionEvaluator evaluator(scriptingService);
    ExpressionResult res = EvaluateNextExpression(text, Types::TypeId<T>(), evaluator, startIndex);
    EXPECT_EQ(expectedStart, res.GetStartOfExpression());
    EXPECT_EQ(expectedEnd, res.GetEndOfExpression());
    ASSERT_TRUE(res.GetValue().IsConvertibleTo<T>());
    EXPECT_EQ(expectedValue, res.GetValue().As<T>());
    EXPECT_EQ(isEnd, IsRemainderEmpty(text, res.GetEndOfExpression()));
}

TEST(ExpressionEvaluationTests, TestEvaluateDouble)
{
    ScopedServiceProvider provider;
    auto scriptingService = ScriptingServiceFactory().CreateScriptingService();
    TestExpression(scriptingService, "3.14", 0, 0, 4, 3.14, true);
    TestExpression(scriptingService, "  3.14", 0, 2, 6, 3.14, true);
    TestExpression(scriptingService, "3.14  ", 0, 0, 4, 3.14, true);
    TestExpression(scriptingService, "3.14 m", 0, 0, 4, 3.14, false);
    TestExpression(scriptingService, "3.14 19.8*0.001", 4, 5, 15, 0.0198, true);
    TestExpression(scriptingService, "-3.14  ", 0, 0, 5, -3.14, true);
    TestExpression(scriptingService, "-3  ", 0, 0, 2, -3., true);
}

TEST(ExpressionEvaluationTests, TestEvaluateQuantity)
{
    ScopedServiceProvider provider;
    auto scriptingService = ScriptingServiceFactory().CreateScriptingService();
    TestExpression(scriptingService, "3.14", 0, 0, 4, Length(3.14), true);
    TestExpression(scriptingService, "-3", 0, 0, 2, Length(-3), true);
    TestExpression(scriptingService, "3.14 km", 0, 0, 7, Length(3140), true);
    TestExpression(scriptingService, "  3.14 km", 0, 2, 9, Length(3140), true);
    TestExpression(scriptingService, "3.14 km   ", 0, 0, 7, Length(3140), true);
    TestExpression(scriptingService, "3.14 kg   ", 0, 0, 4, Length(3.14), false);
    TestExpression(scriptingService, "3.14 m 19.8mm", 6, 7, 13, Length(0.0198), true);
}

TEST(ExpressionEvaluationTests, IsLookupExpressionTests)
{
    ScopedServiceProvider provider;
    auto scriptingService = ScriptingServiceFactory().CreateScriptingService();
    EXPECT_TRUE(scriptingService->IsLookupExpression("vec;"));
    EXPECT_FALSE(scriptingService->IsLookupExpression("vec(1,2);"));
    EXPECT_TRUE(scriptingService->IsLookupExpression("vec.f(1,2);"));
    EXPECT_FALSE(scriptingService->IsLookupExpression("a+b;"));
}
