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
#include "jsValue_delegate.h"
#include "EliminateWrappersConversion.h"
#include "Reflection\Classes\Class.h"
#include "value.Reflection.h"
#include "reflected_value.h"
#include "jsScript\jsStack.h"
#include "..\Reflection\ObjectAsJsValue.h"
#include "jsValue_value.h"
#include "ConversionHelper.h"
#include "ref_ptr.Reflection.h"
#include "ixlib_iterator.hpp"
#include "ixlib_javascript.hh"
namespace ixion { namespace javascript {
    using DNVS::MoFa::Reflection::Objects::Object;

    jsValue_delegate::jsValue_delegate(const mofa::ref<jsValue>& object, const mofa::ref<jsValue>& method)
        : m_object(object)
        , m_method(method)
    {
    }

    jsValue_delegate::~jsValue_delegate()
    {

    }

    value::value_type jsValue_delegate::getType() const
    {
        return value::VT_TYPE;
    }

    ixion::ref_ptr<value> jsValue_delegate::eliminateWrappers()
    {
        ref_ptr<value> result = CreateObject();
        if (result)
            return result;
        else
            return this;
    }

    ixion::ref_ptr<value> jsValue_delegate::duplicate()
    {
        return new jsValue_delegate(m_object, m_method);
    }

    ixion::ref_ptr<value> jsValue_delegate::duplicateAndResolveDelegate()
    {
        return eliminateWrappers();
    }

    std::string jsValue_delegate::GetErrorContext() const
    {
        std::string error = "";

        std::string name;
        if (m_object && m_object->getName(name))
        {
            error += "Identifier \'" + name + "\'";
        }

        std::string identifier;
        if (m_object && m_method && m_object->methodName(m_method, identifier))
        {
            if (error.size() != 0)
                error += ", ";
            else
                error += "In ";

            error += m_object->typeName();
            error += " \'" + identifier + "\' : ";
        }
        else
            error += ": ";
        return error;
    }

    ref_ptr<value> jsValue_delegate::CreateObject() const
    {
        try {
            return ConversionHelper::ToIxion(m_method->duplicate(m_object));
        }
        catch (...)
        {
            HandleExceptions();
            throw;
        }
    }

    ref_ptr<value> jsValue_delegate::lookup(std::string const &identifier)
    {
        ref_ptr<value> result = CreateObject();
        if (result)
            return result->lookup(identifier);
        else
            return value::lookup(identifier);
    }

    ref_ptr<value> jsValue_delegate::assign(ref_ptr<value> op2)
    {
        try {
            auto typeLibrary = jsStack::stack()->GetTypeLibrary();
            auto object = Objects::Object(typeLibrary, op2).ConvertToDynamicType();
            mofa::ref<jsValue> result = m_method->assign(m_object, object.As<mofa::ref<jsValue>>());
            return ConversionHelper::ToIxion(result);
        }
        catch (...)
        {
            HandleExceptions();
            throw;
        }
    }

    ref_ptr<value> jsValue_delegate::call(parameter_list const &parameters)
    {        
        std::vector<mofa::ref<jsValue>> stack(parameters.size() + 1);
        jsValue::Params args(parameters.size() + 1);
        args[0] = m_object;
        auto typeLibrary = jsStack::stack()->GetTypeLibrary();
        for (size_t i = 0; i < parameters.size(); ++i)
        {
            auto object = Object(typeLibrary, parameters[i]);
            mofa::ref<jsValue> result = object.As<mofa::ref<jsValue>>();
            args[i + 1] = result;
            stack[i + 1] = result;
        }
        return ConversionHelper::ToIxion(m_method->call(args));
    }

    ixion::ref_ptr<value> jsValue_delegate::construct(parameter_list const& parameters)
    {
        return call(parameters);
    }

    ixion::ref_ptr<value> jsValue_delegate::operatorBinary(operator_id op, ref_ptr<value> op2) const
    {
        ref_ptr<value> val = CreateObject();
        if (val)
            return val->operatorBinary(op, op2);
        else
            return ixion::javascript::value::operatorBinary(op, op2);
    }

    ref_ptr<value> jsValue_delegate::operatorUnary(operator_id op) const
    {
        ref_ptr<value> val = CreateObject();
        if (val)
            return val->operatorUnary(op);
        else
            return value::operatorUnary(op);
    }

    ref_ptr<value> jsValue_delegate::operatorBinaryShortcut(operator_id op, expression const &op2, context const &ctx) const
    {
        ref_ptr<value> val = CreateObject();
        if (val)
            return val->operatorBinaryShortcut(op, op2, ctx);
        else
            return value::operatorBinaryShortcut(op, op2, ctx);
    }

    ref_ptr<value> jsValue_delegate::operatorUnaryModifying(operator_id op)
    {
        ref_ptr<value> val = CreateObject();
        if (val)
            return val->operatorUnaryModifying(op);
        else
            return value::operatorUnaryModifying(op);
    }

    ref_ptr<value> jsValue_delegate::operatorBinaryModifying(operator_id op, ref_ptr<value> op2)
    {
        ref_ptr<value> val = CreateObject();
        if (val)
            return val->operatorBinaryModifying(op, op2);
        else
            return value::operatorBinaryModifying(op, op2);
    }


    ixion::ref_ptr<value> jsValue_delegate::subscript(value const &index)
    {
        ref_ptr<value> val = CreateObject();
        if (val)
            return val->subscript(index);
        else
            return value::subscript(index);
    }

    ixlib_iterator jsValue_delegate::begin() const
    {
        ref_ptr<value> val = CreateObject();
        if (val)
            return val->begin();
        else
            return value::begin();
    }

    ixlib_iterator jsValue_delegate::end() const
    {
        ref_ptr<value> val = CreateObject();
        if (val)
            return val->end();
        else
            return value::end();
    }

    std::size_t jsValue_delegate::size() const
    {
        ref_ptr<value> val = CreateObject();
        if (val)
            return val->size();
        else
            return value::size();
    }

    std::string jsValue_delegate::toString() const
    {
        ref_ptr<value> val = CreateObject();
        if (val)
            return val->toString();
        else
            return value::toString();

    }

    int jsValue_delegate::toInt() const
    {
        ref_ptr<value> val = CreateObject();
        if (val)
            return val->toInt();
        else
            return value::toInt();
    }

    double jsValue_delegate::toFloat(bool allow_throw /*= false*/) const
    {
        ref_ptr<value> val = CreateObject();
        if (val)
            return val->toFloat(allow_throw);
        else
            return value::toFloat(allow_throw);
    }

    bool jsValue_delegate::toBoolean() const
    {
        ref_ptr<value> val = CreateObject();
        if (val)
            return val->toBoolean();
        else
            return value::toBoolean();
    }

    void DoReflect(TypeLibraryPointer typeLibrary, jsValue_delegate**)
    {
        using namespace Classes;
        Class<jsValue_delegate, Public<value>, ref_ptr<jsValue_delegate, value>> cls(typeLibrary, "jsValue_delegate");
        typeLibrary->GetConversionGraph()->AddConversion(
            Types::TypeId<jsValue_delegate>(), Types::TypeId<void>(),
            TypeConversions::ConversionType::DynamicTypeConversion,
            std::make_shared<EliminateWrappersConversion>());
    }

}}