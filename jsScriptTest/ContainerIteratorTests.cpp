#include <gtest/gtest.h>

#include "Reflection/Classes/Class.h"
#include "Reflection/Objects/Delegate.h"
#include <Reflection/Containers/ReflectList.h>
#include "Services/ScopedServiceProvider.h"
#include "jsScript/jsArray.h"
#include "jsScript/jsStack.h"
#include "jsScript/jsArrayToContainerAlternativeConverter.h"
#include "ScriptingServiceFactory.h"

class ClassWithContainer {
public:
    ClassWithContainer() 
        : m_args({ 1,2,5 })
    {
    }
    ClassWithContainer(const std::list<int>& args) : m_args(args) {}
    const std::list<int>& GetMyList() const { return m_args; }
private:
    std::list<int> m_args;
};

void DoReflect(DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer typeLibrary, ClassWithContainer**)
{
    using namespace DNVS::MoFa::Reflection::Classes;
    Class<ClassWithContainer> cls(typeLibrary, "ClassWithContainer");
    cls.Constructor();
    cls.Constructor<const std::list<int>&>(Explicit);
    cls.Get("MyList", &ClassWithContainer::GetMyList);
}

using namespace DNVS::MoFa::Scripting;

class ContainerIteratorTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_scriptingService = ScriptingServiceFactory().CreateScriptingService();
    }

    DNVS::MoFa::Services::ScopedServiceProvider m_sp;
    ScriptingServicePointer m_scriptingService;
};

TEST_F(ContainerIteratorTests, ReflectMemberWithContainer_IterateThroughContainer)
{
    DNVS::MoFa::Reflection::Reflect<ClassWithContainer>(m_scriptingService->GetTypeLibrary());
    m_scriptingService->Execute("c1 = ClassWithContainer();");
    m_scriptingService->Execute(
        "var result = 0;"
        "for(var a in c1.MyList) {\n"
        "    result+=a;"
        "}");
    EXPECT_EQ(8, m_scriptingService->Execute("result;").As<int>());
}

TEST_F(ContainerIteratorTests, PassArrayIn_ConvertToList)
{
    DNVS::MoFa::Reflection::Reflect<ClassWithContainer>(m_scriptingService->GetTypeLibrary());
    m_scriptingService->Execute("c1 = ClassWithContainer(Array(1,2,5,7));");
    m_scriptingService->Execute(
        "var result = 0;"
        "for(var a in c1.MyList) {\n"
        "    result+=a;"
        "}");
    EXPECT_EQ(15, m_scriptingService->Execute("result;").As<int>());
}

TEST_F(ContainerIteratorTests, ConvertArrayToListOfInts)
{
    DNVS::MoFa::Reflection::Reflect<ClassWithContainer>(m_scriptingService->GetTypeLibrary());
    DNVS::MoFa::Reflection::AutoReflect<std::list<ClassWithContainer>>::Reflect(m_scriptingService->GetTypeLibrary());
    auto object = m_scriptingService->Execute("c1 = Array(1,2,5,7);");
    jsArrayToContainerAlternativeConverter converter(m_scriptingService->GetTypeLibrary());
    auto array = object.ConvertTo<mofa::ref<jsArray>>();
    auto variant = array.GetVariant();
    EXPECT_TRUE(converter.CanConvert(variant, DNVS::MoFa::Reflection::Types::TypeId<const std::list<int>&>()));
    EXPECT_FALSE(converter.CanConvert(variant, DNVS::MoFa::Reflection::Types::TypeId<double>()));
    EXPECT_FALSE(converter.CanConvert(variant, DNVS::MoFa::Reflection::Types::TypeId<const std::list<ClassWithContainer>>()));
    DNVS::MoFa::Reflection::Objects::Object container(m_scriptingService->GetTypeLibrary(), converter.CreateConverter(DNVS::MoFa::Reflection::Types::TypeId<const std::list<int>&>())->Convert(variant));
    ASSERT_TRUE(container.IsConvertibleTo<const std::list<int>&>());
    std::list<int> myList = container.As<const std::list<int>&>();
    EXPECT_EQ(std::list<int>({ 1,2,5,7 }), myList);
}

