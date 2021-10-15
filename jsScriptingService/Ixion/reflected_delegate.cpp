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
#include "reflected_delegate.h"
#include "reflected_value.h"
#include "Reflection\Objects\Delegate.h"
#include "Reflection\Members\MemberWithArguments.h"
#include "value.Reflection.h"
#include "EliminateWrappersConversion.h"
#include "Reflection\Classes\Class.h"
#include "ConversionHelper.h"
#include "ref_ptr.Reflection.h"
#include "ixlib_iterator.hpp"
#include "Scripting/INameService.h"
namespace ixion { namespace javascript {
    using DNVS::MoFa::Reflection::Members::MemberType;
    using namespace DNVS::MoFa::Scripting;

    reflected_delegate::reflected_delegate(const Object& object, const std::string& name, const Object& jsObject)
        : m_object(object)
        , m_jsObject(jsObject)
        , m_name(name)
    {
    }

    reflected_delegate::~reflected_delegate()
    {

    }

    value::value_type reflected_delegate::getType() const
    {
        return VT_TYPE;
    }

    ixlib_iterator reflected_delegate::begin() const
    {
        ref_ptr<value> val = CreateObject();
        if (val)
            return val->begin();
        else
            return value::begin();
    }

    ixlib_iterator reflected_delegate::end() const
    {
        ref_ptr<value> val = CreateObject();
        if (val)
            return val->end();
        else
            return value::end();
    }

    std::size_t reflected_delegate::size() const
    {
        ref_ptr<value> val = CreateObject();
        if (val)
            return val->size();
        else
            return value::size();
    }

    ref_ptr<value> reflected_delegate::eliminateWrappers()
    {
        ref_ptr<value> result = CreateObject();
        if (result)
            return result;
        else
            return this;
    }

    ref_ptr<value> reflected_delegate::duplicate()
    {
        return new reflected_delegate(m_object, m_name);
    }

    ref_ptr<value> reflected_delegate::duplicateAndResolveDelegate()
    {
        return eliminateWrappers();
    }

    ref_ptr<value> reflected_delegate::operatorBinary(operator_id op, ref_ptr<value> op2) const
    {
        ref_ptr<value> val = CreateObject();
        if (val)
            return val->operatorBinary(op, op2);
        else
            return ixion::javascript::value::operatorBinary(op, op2);
    }

    ref_ptr<value> reflected_delegate::operatorUnary(operator_id op) const
    {
        ref_ptr<value> val = CreateObject();
        if (val)
            return val->operatorUnary(op);
        else
            return value::operatorUnary(op);
    }

    ref_ptr<value> reflected_delegate::operatorBinaryShortcut(operator_id op, expression const &op2, context const &ctx) const
    {
        ref_ptr<value> val = CreateObject();
        if (val)
            return val->operatorBinaryShortcut(op, op2, ctx);
        else
            return value::operatorBinaryShortcut(op, op2, ctx);
    }

    ref_ptr<value> reflected_delegate::operatorUnaryModifying(operator_id op)
    {
        ref_ptr<value> val = CreateObject();
        if (val)
            return val->operatorUnaryModifying(op);
        else
            return value::operatorUnaryModifying(op);
    }

    ref_ptr<value> reflected_delegate::operatorBinaryModifying(operator_id op, ref_ptr<value> op2)
    {
        ref_ptr<value> val = CreateObject();
        if (val)
            return val->operatorBinaryModifying(op, op2);
        else
            return value::operatorBinaryModifying(op, op2);
    }

    ref_ptr<value> reflected_delegate::lookup(std::string const &identifier)
    {
        ref_ptr<value> val = CreateObject();
        if (val)
            return val->lookup(identifier);
        else
            return value::lookup(identifier);
    }

    ixion::ref_ptr<value> reflected_delegate::subscript(value const &index)
    {
        ref_ptr<value> val = CreateObject();
        if (val)
            return val->subscript(index);
        else
            return value::subscript(index);
    }

    ref_ptr<value> reflected_delegate::assign(ref_ptr<value> op2)
    {
        try {
            return ConversionHelper::ToIxion(m_object.Invoke(m_name, { Object(m_object.GetTypeLibrary(), op2).ConvertToDynamicType() }));
        }
        catch (...)
        {
            HandleExceptions();
            throw;
        }
    }

    ref_ptr<value> reflected_delegate::call(parameter_list const& parameters)
    {
        std::vector<Object> args(parameters.size());

        for (size_t i = 0; i < parameters.size(); ++i)
            args[i].Reset(Object(m_object.GetTypeLibrary(), parameters[i]).ConvertToDynamicType());

        return ConversionHelper::ToIxion(m_object.Invoke(m_name, args));
    }

    ixion::ref_ptr<value> reflected_delegate::construct(parameter_list const& parameters)
    {
        return call(parameters);
    }

    Delegate reflected_delegate::GetDelegate() const
    {
        return m_object.Lookup(m_name);
    }

    std::string reflected_delegate::toString() const
    {
        ref_ptr<value> val = CreateObject();
        if (val)
            return val->toString();
        else
            return value::toString();
    }
    int reflected_delegate::toInt() const
    {
        ref_ptr<value> val = CreateObject();
        if (val)
            return val->toInt();
        else
            return value::toInt();
    }

    double reflected_delegate::toFloat(bool allow_throw /*= false*/) const
    {
        ref_ptr<value> val = CreateObject();
        if (val)
            return val->toFloat(allow_throw);
        else
            return value::toFloat(allow_throw);
    }
    bool reflected_delegate::toBoolean() const
    {
        ref_ptr<value> val = CreateObject();
        if (val)
            return val->toBoolean();
        else
            return value::toBoolean();
    }

    std::string reflected_delegate::GetErrorContext() const
    {
        std::string error = "";
        if (m_object.GetTypeLibrary())
        {
            auto service = m_object.GetTypeLibrary()->GetServiceProvider().TryGetService<INameService>();
            if (service && service->HasName(m_object))
                error += "Identifier '" + service->GetName(m_object) + "'";
        }
        if (error.size() != 0)
            error += ", ";
        else
            error += "In ";

        error += " '" + m_name + "' : ";
        return error;
    }

    ref_ptr<value> reflected_delegate::CreateObject() const
    {
        try {
            auto member = m_object.PrepareInvoke(m_name, {}, MemberType::TypePropertyGet);

            if (member && member->IsOk())
                return ConversionHelper::ToIxion(Object(m_object.GetTypeLibrary(), member->Invoke()));
            else
                return nullptr;
        }
        catch (...)
        {
            HandleExceptions();
            throw;
        }
    }

    void DoReflect(TypeLibraryPointer typeLibrary, reflected_delegate**)
    {
        using namespace Classes;
        Class<reflected_delegate, Public<value>, ref_ptr<reflected_delegate, value>> cls(typeLibrary, "reflected_delegate");
        typeLibrary->GetConversionGraph()->AddConversion(
            Types::TypeId<reflected_delegate>(), Types::TypeId<void>(),
            TypeConversions::ConversionType::DynamicTypeConversion,
            std::make_shared<EliminateWrappersConversion>());
        cls.ImplicitConversion([](const reflected_delegate& input)
        {
            return input.GetDelegate().AsMember();
        });

    }

}}
