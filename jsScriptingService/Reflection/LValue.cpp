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
#include "LValue.h"
#include "Reflection\Classes\Class.h"
#include "Reflection\Objects\InvokeBinaryOperator.h"
#include "Reflection\Objects\InvokeUnaryOperator.h"
#include "Undefined.h"
#include "ILValueService.h"

namespace DNVS { namespace MoFa { namespace Scripting {
    using namespace DNVS::MoFa::Operators::Tags;
    using namespace DNVS::MoFa::Reflection::Objects;

    LValue::LValue(const Object& object)
    {
        Reset(object);
        AddReference();
    }

    LValue::LValue()
        : m_object(nullptr, Undefined())
    {

    }

    LValue::LValue(const LValue& other)
        : m_object(other.m_object)
    {
        AddReference();
    }

    LValue::LValue(LValue&& other)
        : m_object(other.m_object)
    {
        other.Reset(Object());
        AddReference();
    }

    LValue::~LValue()
    {
        RemoveReference();
    }

    LValue& LValue::operator=(const LValue& object)
    {
        Reset(object.GetObject());
        AddReference();
        return *this;
    }

    Object LValue::operator!=(const Object& other) const
    {
        return m_object != other;
    }

    TypeLibraryPointer GetTypeLibrary(const Object& lhs)
    {
        if (lhs.GetTypeLibrary())
            return lhs.GetTypeLibrary();
        throw std::runtime_error("Type library not found");
    }


    TypeLibraryPointer GetTypeLibrary(const Object& lhs, const Object& rhs)
    {
        if (lhs.GetTypeLibrary())
            return lhs.GetTypeLibrary();
        if (rhs.GetTypeLibrary())
            return rhs.GetTypeLibrary();
        throw std::runtime_error("Type library not found");
    }

    LValue& LValue::operator=(const Object& other)
    {
        auto typeLibrary = GetTypeLibrary(m_object, other);
        auto member = PrepareInvokeBinaryModifyingOperator<Assign>(typeLibrary, m_object.GetVariant(), other.GetVariant());
        if (member && member->IsOk())
        {
            member->Invoke();
        }
        else
            Reset(other);
        return *this;
    }
    
    LValue& LValue::operator+=(const Object& other)
    {
        auto typeLibrary = GetTypeLibrary(m_object, other);
        auto member = PrepareInvokeBinaryModifyingOperator<PlusAssign>(typeLibrary, m_object.GetVariant(), other.GetVariant());
        if (member && member->IsOk())
        {
            member->Invoke();
        }
        else
            Reset(m_object + other);
        
        return *this;
    }

    LValue& LValue::operator-=(const Object& other)
    {
        auto typeLibrary = GetTypeLibrary(m_object, other);
        auto member = PrepareInvokeBinaryModifyingOperator<MinusAssign>(typeLibrary, m_object.GetVariant(), other.GetVariant());
        if (member && member->IsOk())
        {
            member->Invoke();
        }
        else
            Reset(m_object - other);
        return *this;
    }

    LValue& LValue::operator*=(const Object& other)
    {
        auto typeLibrary = GetTypeLibrary(m_object, other);
        auto member = PrepareInvokeBinaryModifyingOperator<MultipliesAssign>(typeLibrary, m_object.GetVariant(), other.GetVariant());
        if (member && member->IsOk())
        {
            member->Invoke();
        }
        else
            Reset(m_object * other);
        return *this;
    }

    LValue& LValue::operator/=(const Object& other)
    {
        auto typeLibrary = GetTypeLibrary(m_object, other);
        auto member = PrepareInvokeBinaryModifyingOperator<DividesAssign>(typeLibrary, m_object.GetVariant(), other.GetVariant());
        if (member && member->IsOk())
        {
            member->Invoke();
        }
        else
            Reset(m_object / other);
        return *this;
    }

    LValue& LValue::operator%=(const Object& other)
    {
        auto typeLibrary = GetTypeLibrary(m_object, other);
        auto member = PrepareInvokeBinaryModifyingOperator<ModulusAssign>(typeLibrary, m_object.GetVariant(), other.GetVariant());
        if (member && member->IsOk())
        {
            member->Invoke();
        }
        else
            Reset(m_object % other);
        return *this;
    }
    
    LValue& LValue::operator++()
    {
        auto typeLibrary = GetTypeLibrary(m_object);
        auto member = PrepareInvokeUnaryModifyingOperator<PostInc>(typeLibrary, m_object.GetVariant());
        if (member && member->IsOk())
        {
            member->Invoke();
        }
        else
            Reset(m_object + Object(typeLibrary, 1));
        return *this;
    }

    LValue LValue::operator++(int)
    {
        LValue self(m_object);
        operator++();
        return self;
    }

    LValue& LValue::operator--()
    {
        auto typeLibrary = GetTypeLibrary(m_object);
        auto member = PrepareInvokeUnaryModifyingOperator<PostDec>(typeLibrary, m_object.GetVariant());
        if (member && member->IsOk())
        {
            member->Invoke();
        }
        else
            Reset(m_object - Object(typeLibrary, 1));
        return *this;
    }

    LValue LValue::operator--(int)
    {
        LValue self(m_object);
        operator--();
        return self;
    }

    const DNVS::MoFa::Scripting::Object& LValue::GetObject() const
    {
        return m_object;
    }

    void LValue::Reset(const Object& other)
    {
        RemoveReference();
        m_object = other.WrapValue().ToLValue();
        AddReference();
    }

    void LValue::Reset()
    {
        RemoveReference();
        m_object = Object(nullptr, Undefined());
    }

    void LValue::RemoveReference()
    {
        if (m_object.GetTypeLibrary())
        {
            auto service = m_object.GetTypeLibrary()->GetServiceProvider().TryGetService<ILValueService>();
            if (service)
                service->RemoveReference(*this);
        }            
    }

    void LValue::AddReference()
    {
        if (m_object.GetTypeLibrary())
        {
            auto service = m_object.GetTypeLibrary()->GetServiceProvider().TryGetService<ILValueService>();
            if (service)
                service->AddReference(*this);
        }
    }

    using namespace Reflection;
    class ObjectFromLValue : public TypeConversions::IConversion
    {
    public:
        virtual Variants::Variant Convert(const Variants::Variant& other)
        {
            const LValue& wrapper = Variants::InternalVariantService::UnreflectUnchecked<const LValue&>(other);
            return wrapper.GetObject().GetVariant();
        }

        virtual void IntrusiveConvert(Variants::Variant& variable)
        {
            variable = Convert(variable);
        }
    };
    void DoReflect(const TypeLibraryPointer& typeLibrary, LValue**)
    {
        using namespace Reflection::Classes;
        Class<LValue,LValue> cls(typeLibrary, "LValue");        
        cls.Constructor<const Object&>();
        cls.Operator(This = Object());
        cls.Operator(This += Object());
        cls.Operator(This -= Object());
        cls.Operator(This /= Object());
        cls.Operator(This *= Object());
        cls.Operator(This %= Object());
        cls.Operator(++This);
        cls.Operator(--This);
        cls.Operator(This++);
        cls.Operator(This--);
        typeLibrary->GetConversionGraph()->AddConversion(Types::TypeId<LValue>(), Types::TypeId<void>(), TypeConversions::ConversionType::DynamicTypeConversion, std::make_shared<ObjectFromLValue>());
    }

}}}

