//one line to give the library's name and an idea of what it does.
// Copyright(C) 2021 DNV AS
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; 
// version 2 of the License.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the GNU
// Library General Public License for more details.
// 
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
// Boston, MA  02110 - 1301, USA.
#include "Null.h"
#include "Reflection\Classes\Class.h"
#include "Reflection\TypeConversions\IConversion.h"
#include "Reflection\TypeConversions\IAlternativeConverter.h"
#include "Reflection\Types\DynamicTypeTraits.h"

using namespace DNVS::MoFa::Reflection;
class NullPtrConverter : public TypeConversions::IConversion
{
public:
    NullPtrConverter(const Types::DecoratedTypeInfo& info)
        : m_info(info)
    {}
    virtual Variants::Variant Convert(const Variants::Variant& variable) override
    {
        return Variants::Variant(nullptr, m_info);
    }

    virtual void IntrusiveConvert(Variants::Variant& variable) override
    {
        variable = Convert(variable);
    }
private:
    Types::DecoratedTypeInfo m_info;
};

bool operator==(std::nullptr_t, const Variants::Variant& v)
{
    if (Types::IsPointer(v.GetDecoratedTypeInfo()))
    {
        return v.GetData() == nullptr;
    }
    else
        return false;
}
bool operator==(const Variants::Variant& v, std::nullptr_t)
{
    return operator==(nullptr, v);
}

bool operator!=(std::nullptr_t, const Variants::Variant& v)
{
    if (Types::IsPointer(v.GetDecoratedTypeInfo()))
    {
        return v.GetData() != nullptr;
    }
    else
        return true;
}

bool operator!=(const Variants::Variant& v, std::nullptr_t)
{
    return operator!=(nullptr, v);
}

class NullPtrAlternativeConverter : public TypeConversions::IAlternativeConverter
{
public:
    virtual bool CanConvert(const Variants::Variant& variant, const Types::DecoratedTypeInfo& returnType) const override
    {
        return Types::IsPointer(returnType);
    }

    virtual TypeConversions::ConversionPointer CreateConverter(const Types::DecoratedTypeInfo& returnType) const override
    {
        return std::make_shared<NullPtrConverter>(returnType);
    }

    virtual TypeConversions::ConversionType::Type GetConversionType() const override
    {
        return TypeConversions::ConversionType::TrivialConversion;
    }

};

namespace std {
    void DoReflect(const DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer& typeLibrary, nullptr_t**)
    {
        using namespace DNVS::MoFa::Reflection;
        using namespace DNVS::MoFa::Reflection::Classes;
        Class<std::nullptr_t> cls(typeLibrary, "null");
        cls.Operator(This.Const == This.Const);
        cls.Operator(This.Const < This.Const);
        cls.Operator(This.Const > This.Const);
        cls.Operator(This.Const <= This.Const);
        cls.Operator(This.Const >= This.Const);
        cls.Operator(This.Const != This.Const);
        cls.Operator(This.Const + This.Const, [](std::nullptr_t, std::nullptr_t) {return 0; });
        cls.Operator(This.Const - This.Const, [](std::nullptr_t, std::nullptr_t) {return 0; });
        cls.Operator(This.Const * This.Const, [](std::nullptr_t, std::nullptr_t) {return 0; });
        cls.Operator(This.Const / This.Const, [](std::nullptr_t, std::nullptr_t) {return std::numeric_limits<double>::quiet_NaN(); });
        cls.Operator(!This.Const);
        cls.Operator(-This.Const, [](std::nullptr_t) {return 0; });
        cls.Operator(This.Const == Variants::Variant());
        cls.Operator(Variants::Variant() == This.Const);
        cls.Operator(This.Const != Variants::Variant());
        cls.Operator(Variants::Variant() != This.Const);
        typeLibrary->GetConversionGraph()->AddAlternativeConverter(Types::TypeId<std::nullptr_t>(), std::make_shared<NullPtrAlternativeConverter>());
        cls.ImplicitConversion([](const nullptr_t&) {return 0; });
        cls.ImplicitConversion([](const nullptr_t&) {return std::string("null"); });
        cls.Function("toString", [](const nullptr_t&) {return std::string("null"); });

        Class<Members::GlobalType> global(typeLibrary, "");
        global.StaticFunction("isNaN", [](const std::nullptr_t&) {return false; });
    }
}
