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
#include "ObjectAsJsValue.h"
#include "Reflection\Classes\Class.h"
#include "Reflection\TypeConversions\IAlternativeConverter.h"
#include "jsScript\Reflection\jsValue.Reflection.h"
#include "jsScript\jsStack.h"
#include "jsScript\jsTypeLibrary.h"


namespace DNVS { namespace MoFa { namespace Scripting {
    using Reflection::Objects::Object;
    ObjectAsJsValue::ObjectAsJsValue(const Reflection::Objects::Object& object)
        : m_object(object)
    {
    }

    Reflection::Objects::Object ObjectAsJsValue::GetObject() const
    {
        return m_object;
    }

    jsValue* ObjectAsJsValue::operator/(jsValue* op2)
    {
        if (dynamic_cast<ObjectAsJsValue*>(op2))
            return (m_object / Object(m_object.GetTypeLibrary(), op2)).As<jsValue*>();
        else
            return op2->InverseDivide(this);
    }

    jsValue* ObjectAsJsValue::operator+(jsValue* op2)
    {
        if (dynamic_cast<ObjectAsJsValue*>(op2))
            return (m_object + Object(m_object.GetTypeLibrary(), op2)).As<jsValue*>();
        else
            return op2->operator+(this);
    }

    jsValue* ObjectAsJsValue::operator-(jsValue* op2)
    {
        if (dynamic_cast<ObjectAsJsValue*>(op2))
            return (m_object - Object(m_object.GetTypeLibrary(), op2)).As<jsValue*>();
        else
            return op2->operator+((-m_object).As<jsValue*>());
    }

    jsValue* ObjectAsJsValue::operator-()
    {
        return (-m_object).As<jsValue*>();
    }

    jsValue* ObjectAsJsValue::operator*(jsValue* op2)
    {
        if (dynamic_cast<ObjectAsJsValue*>(op2))
            return (m_object * Object(m_object.GetTypeLibrary(), op2)).As<jsValue*>();
        else
            return op2->operator *(this);
    }

    jsValue* ObjectAsJsValue::operator==(jsValue* op2)
    {
        if (dynamic_cast<ObjectAsJsValue*>(op2))
            return (m_object == Object(m_object.GetTypeLibrary(), op2)).As<jsValue*>();
        else
            return op2->operator==(this);
    }

    jsValue* ObjectAsJsValue::operator!=(jsValue* op2)
    {
        if (dynamic_cast<ObjectAsJsValue*>(op2))
            return (m_object != Object(m_object.GetTypeLibrary(), op2)).As<jsValue*>();
        else
            return op2->operator!=(this);
    }

    jsValue* ObjectAsJsValue::operator!()
    {
        return (!m_object).As<jsValue*>();
    }

    jsValue* ObjectAsJsValue::operator<=(jsValue* op2)
    {
        if (dynamic_cast<ObjectAsJsValue*>(op2))
            return (m_object <= Object(m_object.GetTypeLibrary(), op2)).As<jsValue*>();
        else
            return op2->operator>(this);
    }

    jsValue* ObjectAsJsValue::operator>=(jsValue* op2)
    {
        if (dynamic_cast<ObjectAsJsValue*>(op2))
            return (m_object >= Object(m_object.GetTypeLibrary(), op2)).As<jsValue*>();
        else
            return op2->operator<(this);
    }

    std::string ObjectAsJsValue::typeName()
    {
        if (m_object.IsValid())
        {
            auto dynamicObject = m_object.ConvertToDynamicType();
            auto type = dynamicObject.GetType();
            if (type && !type->GetName().empty())
                return type->GetName();
            else if (dynamicObject.GetDecoratedTypeInfo().GetTypeInfoPointer())
                return jsStack::stack()->GetJsTypeLibrary().GetTypeName(*dynamicObject.GetDecoratedTypeInfo().GetTypeInfoPointer());
        }
        return "null";
    }

    jsValue* ObjectAsJsValue::operator<(jsValue* op2)
    {
        if (dynamic_cast<ObjectAsJsValue*>(op2))
            return (m_object < Object(m_object.GetTypeLibrary(), op2)).As<jsValue*>();
        else
            return op2->operator>=(this);
    }

    jsValue* ObjectAsJsValue::operator>(jsValue* op2)
    {
        if (dynamic_cast<ObjectAsJsValue*>(op2))
            return (m_object > Object(m_object.GetTypeLibrary(), op2)).As<jsValue*>();
        else
            return op2->operator<=(this);
    }

    class ConvertObjectAsJsValueToVariable : public Reflection::TypeConversions::IConversion
    {
    public:
        virtual Reflection::Variants::Variant Convert(const Reflection::Variants::Variant& other)
        {
            const ObjectAsJsValue* ref = static_cast<const ObjectAsJsValue*>(other.Data());
            return ref->GetObject().GetVariant();
        }
        virtual void IntrusiveConvert(Reflection::Variants::Variant& variable)
        {
            variable = Convert(variable);
        }
    };

    class ObjectToJsValueConverter : public Reflection::TypeConversions::IConversion
    {
    public:
        ObjectToJsValueConverter(const Reflection::TypeLibraries::TypeLibraryPointer& typeLibrary) : m_typeLibrary(typeLibrary) {}

        virtual Reflection::Variants::Variant Convert(const Reflection::Variants::Variant& variable) override
        {
            Reflection::Objects::Object object(m_typeLibrary, variable);
            return Reflection::Variants::VariantService().ReflectType<jsValue*>(new ObjectAsJsValue(object));
        }

        virtual void IntrusiveConvert(Reflection::Variants::Variant& variable) override
        {
            variable = Convert(variable);
        }

    private:
        Reflection::TypeLibraries::TypeLibraryPointer m_typeLibrary;
    };
    class AlternativeObjectToJsValueConverter : public Reflection::TypeConversions::IAlternativeConverter
    {
    public:
        AlternativeObjectToJsValueConverter(const Reflection::TypeLibraries::TypeLibraryPointer& typeLibrary)
            : m_typeLibrary(typeLibrary)
        {}

        virtual bool CanConvert(const Reflection::Variants::Variant& variant, const Reflection::Types::DecoratedTypeInfo& returnType) const override
        {
            if(variant.IsValid() && returnType == Reflection::Types::TypeId<mofa::ref<jsValue>>())
                return true;
            return false;
        }


        virtual Reflection::TypeConversions::ConversionPointer CreateConverter(const Reflection::Types::DecoratedTypeInfo& returnType) const override
        {
            return std::make_shared<ObjectToJsValueConverter>(m_typeLibrary.lock());
        }


        virtual Reflection::TypeConversions::ConversionType::Type GetConversionType() const override
        {
            return Reflection::TypeConversions::ConversionType::UserConversion;
        }

    private:
        std::weak_ptr<Reflection::TypeLibraries::ITypeLibrary> m_typeLibrary;
    };
    void DoReflect(const Reflection::TypeLibraries::TypeLibraryPointer& typeLibrary, ObjectAsJsValue**)
    {
        using namespace Reflection::Classes;
        Class<ObjectAsJsValue, Public<jsValue>, mofa::ref<ObjectAsJsValue>> cls(typeLibrary, "ObjectAsJsValue");
        typeLibrary->GetConversionGraph()->AddConversion(
            Reflection::Types::TypeId<ObjectAsJsValue>(),
            Reflection::Types::TypeId<void>(),
            Reflection::TypeConversions::ConversionType::DynamicTypeConversion,
            std::make_shared<ConvertObjectAsJsValueToVariable>()
        );
        typeLibrary->GetConversionGraph()->AddAlternativeConverter(
            Reflection::Types::TypeId<void>(),
            std::make_shared<AlternativeObjectToJsValueConverter>(typeLibrary)
        );
    }

}}}

