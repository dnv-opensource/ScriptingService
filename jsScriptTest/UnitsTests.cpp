#pragma once
#include "jsUnits/jsCurvature.h"
#include "jsUnits/jsAngle.h"
#include <jsUnits/jsConvertInvalidUnit.h>
#include <jsUnits/jsRotationPerLength.h>
#include "jsUnits/jsRotationalQuadraticDamping.h"
#include "jsUnits/jsCoupledQuadraticDampingMoment.h"
#include "jsUnits/jsRotationalDamping.h"
#include "jsUnits/jsCoupledDamping.h"
#include "jsUnits/jsCoupledQuadraticDampingMoment.h"
#include "jsUnits/jsCoupledQuadraticDampingForce.h"
#include "gmock\gmock.h"
#include <Services/ScopedServiceProvider.h>
#include "Units/Runtime/DefaultInputUnitProvider.h"

#include "Scripting\As.h"
#include <jsUnits/jsLength.h>
#include "ScriptingServiceFactory.h"
#include "Units/ForcePerUnitLength.h"
#include "Reflection/Classes/Class.h"
#include "GoogleTest/gtest_assert_at.h"
#include "jsUnits/jsForce.h"

using namespace DNVS::MoFa::Scripting;
using namespace DNVS::MoFa::Units::Runtime;
using namespace DNVS::MoFa::Units;
using testing::Return;
using testing::_;

class UnitsTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
         m_scriptingService = ScriptingServiceFactory().CreateScriptingService();
         m_sp.RegisterService<IInputUnitProvider>(std::make_shared<DefaultInputUnitProvider>());
    }

    DNVS::MoFa::Services::ScopedServiceProvider m_sp;
    ScriptingServicePointer m_scriptingService;
};

TEST_F(UnitsTests, OneOverLengthNotConvertibleToLength)
{
    ASSERT_THROW(m_scriptingService->Execute("a=Length(1/5m);"), std::exception);
}

TEST_F(UnitsTests, ScriptUnitForcesUnitInitialization)
{
    m_scriptingService->Execute("a=Length(1m);");
}

TEST_F(UnitsTests, ScriptUnitTempGradientUnitInitialization)
{
    m_scriptingService->Execute("a=TempGradient(0.5 delC/mm);");
}

TEST_F(UnitsTests, ScriptAngleDoesNotForceUnitInitialization)
{
    EXPECT_NO_THROW(m_scriptingService->Execute("a=Angle(2deg);"));
}

TEST_F(UnitsTests, jsQuantity_parseFloatOk)
{
    double arg = m_scriptingService->Execute("parseFloat(Length(1));").As<double>();
    EXPECT_DOUBLE_EQ(1, arg);
}

TEST_F(UnitsTests, Quantity_parseFloatOk)
{
    double arg = m_scriptingService->Execute("parseFloat(1m);").As<double>();
    EXPECT_DOUBLE_EQ(1, arg);
}

TEST_F(UnitsTests, DensityTimesAccelerationTimesLengthOk)
{
    EXPECT_NO_THROW(m_scriptingService->Execute("1 ton/m3 * 10 m/s2 * ( 1m);"));
}

TEST_F(UnitsTests, FormattingOfMassAs_kg)
{
    m_scriptingService->Execute("test = 1kg;");
    std::string text = m_scriptingService->Execute("test.toString();").As<std::string>();
    EXPECT_EQ("1 kg", text);
}

TEST_F(UnitsTests, CheckMassDividedWithDensity)
{
    m_scriptingService->Execute("temp_density=1kg/m3;");
    m_scriptingService->Execute("temp_mass=1kg;");
    std::string text = m_scriptingService->Execute("temp_mass/temp_density;").As<std::string>();
    EXPECT_EQ("1 m^3", text);
}

TEST_F(UnitsTests, CheckOneDividedWithDensityWithMass)
{
    m_scriptingService->Execute("temp_density=1kg/m3;");
    m_scriptingService->Execute("temp_mass=1kg;");
    std::string text = m_scriptingService->Execute("1/(temp_density/temp_mass);").As<std::string>();
    EXPECT_EQ("1 m^3", text);
}

TEST_F(UnitsTests, DivideKilometerByMeter_CheckResultIsUnitLess)
{
    double value = m_scriptingService->Execute("1km/1m;").As<double>();
    ASSERT_DOUBLE_EQ(1000, value);
    std::string text = m_scriptingService->Execute("1km/1m;").As<std::string>();
    ASSERT_EQ("1000", text);
}

TEST_F(UnitsTests, SubtractDoubleByUnitLess)
{
    double value = m_scriptingService->Execute("1 - 1m/1m;").As<double>();
    ASSERT_DOUBLE_EQ(0, value);
}
TEST_F(UnitsTests, SubtractUnitLessByDouble)
{
    double value = m_scriptingService->Execute("1m/1m - 1;").As<double>();
    ASSERT_DOUBLE_EQ(0, value);
}

TEST_F(UnitsTests, ConvertUnitToStringAndConcatenate)
{
    m_scriptingService->Execute("a=1 m;");
    EXPECT_NO_THROW(EXPECT_EQ("1 m - Post", m_scriptingService->Execute("a.toString()+' - Post';").As<std::string>()));
    EXPECT_NO_THROW(EXPECT_EQ("Pre - 1 m", m_scriptingService->Execute("'Pre - '+a.toString();").As<std::string>()));
}

TEST_F(UnitsTests, DivideExpressions)
{
    m_scriptingService->Execute("a=1 m;");
    EXPECT_THROW(m_scriptingService->Execute("a.toString()/'2m';").As<std::string>(), std::exception);
}
/*More issues arise from this:
print(temp_density);
-> 7850 Kg/m^3
print(temp_mass);
-> 4710 Kg
print(temp_mass / temp_density);
-> 0.6 Kl

Obviously, the unit should be m^3. This creates continued problems when doing further calculations:
print(1/(temp_density / temp_mass));
-> 0.6 N/m^4*s^2/Kg^-1
Or:
print(1/temp_density / 1/temp_mass);
-> 2.70464e-008 Kg/m^3^-1/Kg
*/


TEST_F(UnitsTests, TestConvertInvalidUnitRotationPerLengthToCurvature)
{
    auto curvatureAsRotationPerLength = m_scriptingService->Execute("1e-006 rad/in;");
    std::string warningText;
    jsCurvature curvtol = jsConvertInvalidUnit<jsRotationPerLength, jsCurvature>(curvatureAsRotationPerLength.As<mofa::ref<jsValue>>(), warningText);

    EXPECT_NE(warningText, "");
    EXPECT_NEAR(3.9370079e-005, curvtol.toDouble(), 1e-8);

}

TEST_F(UnitsTests, TestConvertInvalidUnitAngleToRotationPerLength)
{
    auto RotationPerLengthAsAngle = m_scriptingService->Execute("1e-006 rad;");
    std::string warningText;
    jsRotationPerLength value = jsConvertInvalidUnit<jsAngle, jsRotationPerLength>(RotationPerLengthAsAngle.As<mofa::ref<jsValue>>(), warningText);

    EXPECT_NE(warningText, "");
    EXPECT_NEAR(1e-006, value.toDouble(), 1e-8);

}

TEST_F(UnitsTests, TestConvertInvalidUnitRotationalQuadraticDampingToCoupledQuadraticDampingMoment)
{    
    GetInputUnitSystem().SetUnit(RotationalQuadraticDampingPhenomenon(), "kNs^2m");
    GetInputUnitSystem().SetUnit(CoupledQuadraticDampingMomentPhenomenon(), "Ns^2/m");
    auto CoupledQuadraticDampingMomentAsRotationalQuadraticDamping = m_scriptingService->Execute("10 kN*s^2*in;");
    std::string warningText;
    jsCoupledQuadraticDampingMoment value = jsConvertInvalidUnit<jsRotationalQuadraticDamping, jsCoupledQuadraticDampingMoment>(
        CoupledQuadraticDampingMomentAsRotationalQuadraticDamping.As<mofa::ref<jsValue>>(),
        warningText);

    EXPECT_NE(warningText, "");
    EXPECT_NEAR(254, value.toDouble(), 1e-6);

}

TEST_F(UnitsTests, TestConvertInvalidUnitRotationalDampingToCoupledDamping)
{
    GetInputUnitSystem().SetUnit(RotationalDampingPhenomenon(), "kN*s*m");
    GetInputUnitSystem().SetUnit(CoupledDampingPhenomenon(), "Ns");    
    auto CoupledDampingAsRotationalDamping = m_scriptingService->Execute("10 kN*s*in;");
    std::string warningText;
    jsCoupledDamping value = jsConvertInvalidUnit<jsRotationalDamping, jsCoupledDamping>(
        CoupledDampingAsRotationalDamping.As<mofa::ref<jsValue>>(),
        warningText);

    EXPECT_NE(warningText, "");
    EXPECT_NEAR(254, value.toDouble(), 1e-6);

}

TEST_F(UnitsTests, TestConvertInvalidUnitCoupledQuadraticDampingMomentToCoupledQuadraticDampingForce)
{
    GetInputUnitSystem().SetUnit(CoupledQuadraticDampingMomentPhenomenon(), "kNs^2/m");
    GetInputUnitSystem().SetUnit(CoupledQuadraticDampingForcePhenomenon(), "kNs^2");

    auto CoupledQuadraticDampingMomentAsCoupledQuadraticDampingForce = m_scriptingService->Execute("1e-006 kNs^2/in;");
    std::string warningText;
    jsCoupledQuadraticDampingForce value = jsConvertInvalidUnit<jsCoupledQuadraticDampingMoment, jsCoupledQuadraticDampingForce>(
        CoupledQuadraticDampingMomentAsCoupledQuadraticDampingForce.As<mofa::ref<jsValue>>(),
        warningText);

    EXPECT_NE(warningText, "");
    EXPECT_NEAR(3.9370079e-005, value.toDouble(), 1e-8);

}

TEST_F(UnitsTests, TestComplexInteractionMassAndLength)
{   
    m_scriptingService->Execute("a = 0 Kg*m;");
    m_scriptingService->Execute("b = Mass(37) * Length(37);");
    m_scriptingService->Execute("c = a + b;");
    m_scriptingService->Execute("d = b + a;");
}


TEST_F(UnitsTests, TestParseUnit)
{
    EXPECT_EQ(2.345, jsLength(std::string("2.345")).toDouble());
    EXPECT_EQ(std::numeric_limits<double>::infinity(), jsLength(std::string("+Infinity")).toDouble());
    EXPECT_EQ(-std::numeric_limits<double>::infinity(), jsLength(std::string("-Infinity")).toDouble());
    EXPECT_TRUE(_isnan(jsLength(std::string("NaN"))) != 0);
    EXPECT_NEAR(0.00325, jsLength(std::string("3.25 mm")).toDouble(), 1e-6);
}

TEST_F(UnitsTests, TestOperationOrderOnUnits_SameUnitsRegardlessOfOrder)
{
    EXPECT_EQ("kg*m", m_scriptingService->Execute("Mass(4kg)*Length(2m);").As<jsUnitValue>().GetQuantity().GetUnit().GetUnitName());
    EXPECT_EQ("kg*m", m_scriptingService->Execute("Mass(4kg)*Length(2m)+0kg*m;").As<jsUnitValue>().GetQuantity().GetUnit().GetUnitName());
    EXPECT_EQ("kg*m", m_scriptingService->Execute("0kg*m+Mass(4kg)*Length(2m);").As<jsUnitValue>().GetQuantity().GetUnit().GetUnitName());
}

TEST_F(UnitsTests, TestMultiplyUnits_SimplifyUnit)
{
    EXPECT_EQ("1 m^2", m_scriptingService->Execute("1m * 1m;").As<std::string>());
}

TEST_F(UnitsTests, TestOperationOrderOnUnits_SameUnitsRegardlessOfOrder_ConvertToString)
{
	EXPECT_EQ("8 kg*m", m_scriptingService->Execute("Mass(4kg)*Length(2m);").As<std::string>());
 	EXPECT_EQ("8 kg*m", m_scriptingService->Execute("Mass(4kg)*Length(2m)+0kg*m;").As<std::string>());
 	EXPECT_EQ("8 kg*m", m_scriptingService->Execute("0kg*m+Mass(4kg)*Length(2m);").As<std::string>());
}

TEST_F(UnitsTests, ReservedWords)
{
    EXPECT_THROW(m_scriptingService->Execute("Pressure = Length(37);").As<std::string>(), std::exception);
    EXPECT_NO_THROW(m_scriptingService->Execute("Pressure2 = Length(37);").As<std::string>());
}

DNVS::MoFa::Units::ForcePerUnitLength GetForcePerUnitLength() {
    return DNVS::MoFa::Units::ForcePerUnitLength(1.4);
}
TEST_F(UnitsTests, ConvertFromCoreUnitsToCompatibleCoreUnitViaJsUnits)
{
    using namespace DNVS::MoFa::Reflection::Classes;
    Class<GlobalType> cls(m_scriptingService->GetTypeLibrary(), "");
    cls.StaticFunction("GetForcePerUnitLength", GetForcePerUnitLength);
    EXPECT_NO_THROW(m_scriptingService->Execute("fpu = GetForcePerUnitLength();").As<std::string>());
    DNVS::MoFa::Units::ForcePerUnitLength fpu = m_scriptingService->Execute("fpu;").As<DNVS::MoFa::Units::ForcePerUnitLength>();
}

TEST_F(UnitsTests, ConvertFromStringToUnit)
{
    using namespace DNVS::MoFa::Reflection::Classes;
    EXPECT_NO_THROW_EXCEPTION(m_scriptingService->Execute("\"6N\";").As<DNVS::MoFa::Units::Force>(), std::exception);
    EXPECT_NO_THROW_EXCEPTION(m_scriptingService->Execute("\"6N\";").As<jsForce>(), std::exception);
}

TEST_F(UnitsTests, IsNan)
{
    EXPECT_FALSE(m_scriptingService->Execute("isNaN(123);").As<bool>());
    EXPECT_FALSE(m_scriptingService->Execute("isNaN(-1.23);").As<bool>());
    EXPECT_FALSE(m_scriptingService->Execute("isNaN(5-2);").As<bool>());
    EXPECT_FALSE(m_scriptingService->Execute("isNaN(0);").As<bool>());
    EXPECT_FALSE(m_scriptingService->Execute("isNaN('123');").As<bool>());
    EXPECT_FALSE(m_scriptingService->Execute("isNaN('123 ');").As<bool>());
    EXPECT_TRUE(m_scriptingService->Execute("isNaN('ss123 ');").As<bool>());
    EXPECT_TRUE(m_scriptingService->Execute("isNaN('Hello');").As<bool>());
    EXPECT_TRUE(m_scriptingService->Execute("isNaN('2005/12/12');").As<bool>());
    EXPECT_FALSE(m_scriptingService->Execute("isNaN('');").As<bool>());
    EXPECT_FALSE(m_scriptingService->Execute("isNaN(true);").As<bool>());
    EXPECT_TRUE(m_scriptingService->Execute("isNaN('NaN');").As<bool>());
    EXPECT_TRUE(m_scriptingService->Execute("isNaN(NaN);").As<bool>());
    EXPECT_TRUE(m_scriptingService->Execute("isNaN(0 / 0);").As<bool>());
    EXPECT_TRUE(m_scriptingService->Execute("isNaN(undefined);").As<bool>());
    EXPECT_FALSE(m_scriptingService->Execute("isNaN(null);").As<bool>());
}

TEST_F(UnitsTests, IsFinite)
{
    EXPECT_TRUE(m_scriptingService->Execute("isFinite(123);").As<bool>());
    EXPECT_TRUE(m_scriptingService->Execute("isFinite(-1.23);").As<bool>());
    EXPECT_TRUE(m_scriptingService->Execute("isFinite(5 - 2);").As<bool>());
    EXPECT_TRUE(m_scriptingService->Execute("isFinite(0);").As<bool>());
    EXPECT_TRUE(m_scriptingService->Execute("isFinite('123');").As<bool>());
    EXPECT_FALSE(m_scriptingService->Execute("isFinite('Hello');").As<bool>());
    EXPECT_FALSE(m_scriptingService->Execute("isFinite('2005/12/12');").As<bool>());
    EXPECT_FALSE(m_scriptingService->Execute("isFinite(Infinity);").As<bool>());
    EXPECT_FALSE(m_scriptingService->Execute("isFinite(-Infinity);").As<bool>());
    EXPECT_FALSE(m_scriptingService->Execute("isFinite(0 / 0);").As<bool>());
    EXPECT_TRUE(m_scriptingService->Execute("isFinite(null);").As<bool>());
    EXPECT_FALSE(m_scriptingService->Execute("isFinite(undefined);").As<bool>());
}
