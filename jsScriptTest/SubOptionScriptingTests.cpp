#include "gtest/gtest.h"
#include "Reflection/TypeLibraries/ITypeLibrary.h"
#include "Reflection/Classes/Class.h"
#include "Reflection/Enums/Enum.h"
#include "EnumOrValue.h"
#include "Units/Length.h"
#include "Services/ScopedServiceProvider.h"
#include "Reflection/Members/GlobalType.h"
#include "ScriptingServiceFactory.h"

using namespace DNVS::MoFa::Reflection;
using namespace DNVS::MoFa::Units;

class SubOption 
{
public:
};

void DoReflect(TypeLibraries::TypeLibraryPointer typeLibrary, SubOption**)
{
    using namespace Classes;
    Class<SubOption, std::shared_ptr<SubOption> > cls(typeLibrary, "SubOption");
}

class MemberSubOption : public SubOption
{
public:
};

void DoReflect(TypeLibraries::TypeLibraryPointer typeLibrary, MemberSubOption**)
{
    using namespace Classes;
    Class<MemberSubOption, Public<SubOption>, std::shared_ptr<MemberSubOption> > cls(typeLibrary, "MemberSubOption");
}

template<typename DerivedT, typename BaseT>
class TypedSubOption : public BaseT
{
public:
};

template<typename DerivedT, typename BaseT>
void DoReflect(TypeLibraries::TypeLibraryPointer typeLibrary, TypedSubOption<DerivedT, BaseT>**)
{
    using namespace Classes;
    Class<TypedSubOption<DerivedT,BaseT>, Public<BaseT>, std::shared_ptr<TypedSubOption<DerivedT, BaseT>> > cls(typeLibrary, "");
}

template<typename DerivedT>
class TypedMemberSubOption : public TypedSubOption<DerivedT, MemberSubOption>
{
public:
};

template<typename DerivedT>
void DoReflect(TypeLibraries::TypeLibraryPointer typeLibrary, TypedMemberSubOption<DerivedT>**)
{
    using namespace Classes;
    Class<TypedMemberSubOption<DerivedT>, Public<TypedSubOption<DerivedT, MemberSubOption>>, std::shared_ptr<TypedMemberSubOption<DerivedT>> > cls(typeLibrary, "");
}

class ConeLengthMemOpt : public TypedMemberSubOption<ConeLengthMemOpt>
{
public:
    struct ConeLengthEnum {
        enum Option { Manual = 0, CapMemberLength, GeoConeLength };
    };

    typedef EnumOrValue<ConeLengthEnum::Option, ConeLengthEnum::Manual, Length> ConeLengthOption;

    ConeLengthMemOpt() : m_coneLength(ConeLengthEnum::GeoConeLength) {}
    ConeLengthMemOpt(const ConeLengthMemOpt& other) : m_coneLength(other.m_coneLength) {}
    ConeLengthMemOpt& operator=(const ConeLengthMemOpt& other) 
    {
        m_coneLength = other.m_coneLength;
        return *this;
    }

    void SetConeLength(const ConeLengthOption& length)
    {
        m_coneLength = length;
    }
    const ConeLengthOption& GetConeLength() const
    {
        return m_coneLength;
    }
private:
    ConeLengthOption m_coneLength;
};

void DoReflect(TypeLibraries::TypeLibraryPointer typeLibrary, ConeLengthMemOpt::ConeLengthEnum::Option**)
{
    using namespace Enums;
    Enum<ConeLengthMemOpt::ConeLengthEnum::Option> cls(typeLibrary, "ConeLengthEnum");
    cls.AddFormatter(TrimStart(2) + InsertSpaceBeforeCapitalLetters());
    cls.EnumValue("clConeLength", ConeLengthMemOpt::ConeLengthEnum::GeoConeLength);
    cls.EnumValue("clMemberLength", ConeLengthMemOpt::ConeLengthEnum::CapMemberLength);
}
void DoReflect(TypeLibraries::TypeLibraryPointer typeLibrary, ConeLengthMemOpt**)
{
    using namespace Classes;    
    Class<ConeLengthMemOpt, Public<TypedMemberSubOption<ConeLengthMemOpt>>, std::shared_ptr<ConeLengthMemOpt> > cls(typeLibrary, "ConeLengthMemOpt");
    cls.SetGet("coneLength", &ConeLengthMemOpt::SetConeLength, &ConeLengthMemOpt::GetConeLength);
    cls.Constructor();

    Reflect<ConeLengthMemOpt::ConeLengthEnum::Option>(typeLibrary);
    Reflect<ConeLengthMemOpt::ConeLengthOption>(typeLibrary, "ConeLengthOption");
}

class SubOptionScriptingTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_scriptingService = ScriptingServiceFactory().CreateScriptingService();
    }

    DNVS::MoFa::Services::ScopedServiceProvider m_sp;
    DNVS::MoFa::Scripting::ScriptingServicePointer m_scriptingService;
};

TEST_F(SubOptionScriptingTests, RegisterScripting_ExecuteScriptWithUnits)
{
    Reflect<ConeLengthMemOpt>(m_scriptingService->GetTypeLibrary());
    EXPECT_NO_THROW(m_scriptingService->Execute("a = ConeLengthMemOpt();"));
    EXPECT_NO_THROW(m_scriptingService->Execute("a.coneLength = 28mm;"));
    EXPECT_EQ(ConeLengthMemOpt::ConeLengthOption(Length(0.028)), m_scriptingService->Test("a.coneLength;").As<ConeLengthMemOpt::ConeLengthOption>());
}

TEST_F(SubOptionScriptingTests, RegisterScripting_ExecuteScriptNoUnits)
{
    Reflect<ConeLengthMemOpt>(m_scriptingService->GetTypeLibrary());
    EXPECT_NO_THROW(m_scriptingService->Execute("a = ConeLengthMemOpt();"));
    EXPECT_NO_THROW(m_scriptingService->Execute("a.coneLength = 1.2;"));
    EXPECT_EQ(ConeLengthMemOpt::ConeLengthOption(Length(1.2)), m_scriptingService->Test("a.coneLength;").As<ConeLengthMemOpt::ConeLengthOption>());
}

TEST_F(SubOptionScriptingTests, RegisterScripting_ExecuteScriptEnum)
{
    Reflect<ConeLengthMemOpt>(m_scriptingService->GetTypeLibrary());
    EXPECT_NO_THROW(m_scriptingService->Execute("a = ConeLengthMemOpt();"));
    EXPECT_NO_THROW(m_scriptingService->Execute("a.coneLength = clMemberLength;"));
    EXPECT_EQ(ConeLengthMemOpt::ConeLengthOption(ConeLengthMemOpt::ConeLengthEnum::CapMemberLength), m_scriptingService->Test("a.coneLength;").As<ConeLengthMemOpt::ConeLengthOption>());
}