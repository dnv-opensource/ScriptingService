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
#pragma warning(push)
#pragma warning(disable:4018)
#pragma warning(disable:4804)
#pragma warning(disable:4805)
#include "ConversionHelper.h"
#include "jsScriptingService\Reflection\ObjectAsJsValue.h"
#include "reflected_value.h"
#include "jsValue_value.h"
#include "jsValue_delegate.h"
#include "reflected_delegate.h"
#include "ref_ptr.Reflection.h"
#include "jsScript\jsStack.h"
#include "Reflection\Objects\Delegate.h"
#include "ixlib_iterator.hpp"
#include "reflected_iterator.h"
#include "jsValue_iterator.h"
#include "Operators\Tags.h"
#include "Operators\Invoker.h"
#include "Operators\Stringizer.h"
#include "Reflection\Objects\InvokeBinaryOperator.h"
#include "OptimizedOperatorHandlers.h"
#include "..\Reflection\LValue.h"

namespace ixion { namespace javascript {
    using namespace DNVS::MoFa::Scripting;
    ref_ptr<value> ConversionHelper::ToIxion(const mofa::ref<jsValue>& value)
    {
        if (!value)
            return new reflected_value(DNVS::MoFa::Reflection::Objects::Object(jsStack::stack()->GetTypeLibrary(), nullptr));
        else if (ObjectAsJsValue* result = dynamic_cast<ObjectAsJsValue*>(value.get()))
            return new reflected_value(result->GetObject());
        else
            return new jsValue_value(value);
    }

    ixion::ref_ptr<value> ConversionHelper::ToIxion(const Objects::Object& value)
    {
        if(value.IsConvertibleTo<LValue&>())
            return new reflected_value(value);
        if (!value.IsValid())
            return nullptr;
        auto converted = value.As<mofa::ref<jsValue>>();
        if (auto objectAsJsValue = dynamic_cast<ObjectAsJsValue*>(converted.get()))
        {
            //if value is actually an ObjectAsJsValue, and not just a value that will be converted into ObjectAsJsValue, then extract the underlying data.
            if(converted == value.GetVariant().GetData())
                return new reflected_value(objectAsJsValue->GetObject());
            else
                return new reflected_value(value);
        }            
        else
            return new jsValue_value(converted);
    }

    size_t ConversionHelper::Size(const Objects::Object& value)
    {
        if (value.HasMember("size"))
        {
            return reflected_delegate(value, "size").GetDelegate().As<size_t>();
        }
        auto converted = value.As<mofa::ref<jsValue>>();
        if (dynamic_cast<ObjectAsJsValue*>(converted.get()) == nullptr)
        {
            return Size(converted);
        }
        return 0;
    }

    size_t ConversionHelper::Size(const mofa::ref<jsValue>& value)
    {
        if (value)
            return value->child_size();
        else
            return 0;
    }

    ixlib_iterator ConversionHelper::Begin(const Objects::Object& value)
    {
        if (value.HasMember("begin"))
        {
            auto result = ConversionHelper::Lookup(value, "begin");
            if (result)
                return new reflected_iterator(Objects::Object(value.GetTypeLibrary(), result->call({})).ConvertToDynamicType());
        }
        auto converted = value.As<mofa::ref<jsValue>>();
        if (dynamic_cast<ObjectAsJsValue*>(converted.get()) == nullptr)
        {
            return Begin(converted);
        }
        throw std::runtime_error("Unable to iterate over object - begin not defined");
    }

    ixlib_iterator ConversionHelper::Begin(const mofa::ref<jsValue>& value)
    {
        return new jsValue_iterator(value->child_begin());
    }

    ixlib_iterator ConversionHelper::End(const Objects::Object& value)
    {
        if (value.HasMember("end"))
        {
            auto result = ConversionHelper::Lookup(value, "end");
            if (result)
                return new reflected_iterator(Objects::Object(value.GetTypeLibrary(), result->call({})).ConvertToDynamicType());
        }
        auto converted = value.As<mofa::ref<jsValue>>();
        if (dynamic_cast<ObjectAsJsValue*>(converted.get()) == nullptr)
        {
            return End(converted);
        }
        throw std::runtime_error("Unable to iterate over object - end not defined");
    }

    ixlib_iterator ConversionHelper::End(const mofa::ref<jsValue>& value)
    {
        return new jsValue_iterator(value->child_end());
    }

    template<typename Tag>
    ixion::ref_ptr<value> ConversionHelper::InvokeBinaryOperatorBoth(const Objects::Object& lhs, const Objects::Object& rhs)
    {
        ixion::ref_ptr<value> result = TryOptimizedBinaryOperator<Tag>(lhs, rhs);
        if (result)
            return result;
        using namespace DNVS::MoFa::Operators;
        auto typeLibrary = lhs.GetTypeLibrary();
        if (!typeLibrary)
            typeLibrary = rhs.GetTypeLibrary();
        if(!typeLibrary)
            throw std::runtime_error("Typelibrary not found");
        Members::MemberWithArgumentsPointer delegate = Objects::PrepareInvokeBinaryOperator<Tag>(typeLibrary, lhs.GetVariant(), rhs.GetVariant());
        if (delegate && delegate->IsOk())
            return ConversionHelper::ToIxion(Objects::Object(typeLibrary, delegate->Invoke()));
        auto converted = lhs.As<mofa::ref<jsValue>>();
        if (dynamic_cast<ObjectAsJsValue*>(converted.get()) == nullptr && converted)
        {
            return ConversionHelper::ToIxion(Invoker<Tag>::Invoke(*converted, rhs.As<mofa::ref<jsValue>>()));
        }
        if (!delegate) 
            throw std::runtime_error("No operator overload found for " + Stringizer<Tag>::Stringize(TypeLibraries::GetTypeName(typeLibrary, lhs.GetVariant()), TypeLibraries::GetTypeName(typeLibrary, rhs.GetVariant())));
        else
            throw std::runtime_error("Unable to convert all the arguments for " + Stringizer<Tag>::Stringize(TypeLibraries::GetTypeName(typeLibrary, lhs.GetVariant()), TypeLibraries::GetTypeName(typeLibrary, rhs.GetVariant())));
    }

    template<typename Tag>
    ixion::ref_ptr<value> ConversionHelper::InvokeBinaryOperatorReflection(const Objects::Object& lhs, const Objects::Object& rhs)
    {
        using namespace DNVS::MoFa::Operators;
        auto typeLibrary = lhs.GetTypeLibrary();
        if (!typeLibrary)
            typeLibrary = rhs.GetTypeLibrary();
        if (!typeLibrary)
            throw std::runtime_error("Typelibrary not found");
        Members::MemberWithArgumentsPointer delegate = Objects::PrepareInvokeBinaryOperator<Tag>(typeLibrary, lhs.GetVariant(), rhs.GetVariant());
        if (!delegate)
            throw std::runtime_error("No operator overload found for " + Stringizer<Tag>::Stringize(TypeLibraries::GetTypeName(typeLibrary, lhs.GetVariant()), TypeLibraries::GetTypeName(typeLibrary, rhs.GetVariant())));
        else if(!delegate->IsOk())
            throw std::runtime_error("Unable to convert all the arguments for " + Stringizer<Tag>::Stringize(TypeLibraries::GetTypeName(typeLibrary, lhs.GetVariant()), TypeLibraries::GetTypeName(typeLibrary, rhs.GetVariant())));
        return ConversionHelper::ToIxion(Objects::Object(typeLibrary, delegate->Invoke()));
    }

    ixion::ref_ptr<value> ConversionHelper::InvokeBinaryOperator(const Objects::Object& lhs, const Objects::Object& rhs, value::operator_id op)
    {
        using namespace DNVS::MoFa::Operators::Tags;
        switch (op)
        {
            case value::OP_PLUS:
                return InvokeBinaryOperatorBoth<Plus>(lhs, rhs);
            case value::OP_MINUS:
                return InvokeBinaryOperatorBoth<Minus>(lhs, rhs); 
            case value::OP_MULTIPLY:
                return InvokeBinaryOperatorBoth<Multiplies>(lhs, rhs); 
            case value::OP_DIVIDE:
                return InvokeBinaryOperatorBoth<Divides>(lhs, rhs);
            case value::OP_INVERSE_DIVIDE:
                return InvokeBinaryOperatorBoth<Divides>(rhs, lhs);
            case value::OP_BIT_XOR:
                return InvokeBinaryOperatorReflection<BitwiseXor>(lhs, rhs);
            case value::OP_MODULO:
                return InvokeBinaryOperatorReflection<Modulus>(lhs, rhs);
            case value::OP_BIT_AND:
                return InvokeBinaryOperatorReflection<BitwiseAnd>(lhs, rhs);
            case value::OP_BIT_OR:
                return InvokeBinaryOperatorReflection<BitwiseOr>(lhs, rhs);
            case value::OP_LEFT_SHIFT:
                return InvokeBinaryOperatorReflection<ShiftLeft>(lhs, rhs);
            case value::OP_RIGHT_SHIFT:
                return InvokeBinaryOperatorReflection<ShiftRight>(lhs, rhs);
            case value::OP_LOGICAL_OR:
                return InvokeBinaryOperatorReflection<LogicalOr>(lhs, rhs);
            case value::OP_LOGICAL_AND:
                return InvokeBinaryOperatorReflection<LogicalAnd>(lhs, rhs);
            case value::OP_EQUAL:
                return InvokeBinaryOperatorBoth<EqualTo>(lhs, rhs);
            case value::OP_NOT_EQUAL:
                return InvokeBinaryOperatorBoth<NotEqualTo>(lhs, rhs);
            case value::OP_LESS_EQUAL:
                return InvokeBinaryOperatorBoth<LessEqual>(lhs, rhs);
            case value::OP_GREATER_EQUAL:
                return InvokeBinaryOperatorBoth<GreaterEqual>(lhs, rhs);
            case value::OP_LESS:
                return InvokeBinaryOperatorBoth<Less>(lhs, rhs);
            case value::OP_GREATER:
                return InvokeBinaryOperatorBoth<Greater>(lhs, rhs);
            case value::OP_IDENTICAL:
                if (lhs.ConvertToDynamicType().GetDecoratedTypeInfo().GetTypeInfo() == rhs.ConvertToDynamicType().GetDecoratedTypeInfo().GetTypeInfo())
                    return ConversionHelper::ToIxion(lhs == rhs);
                else
                    return ConversionHelper::ToIxion(Objects::Object(lhs.GetTypeLibrary(), false));
            case value::OP_NOT_IDENTICAL:
                if (lhs.ConvertToDynamicType().GetDecoratedTypeInfo().GetTypeInfo() != rhs.ConvertToDynamicType().GetDecoratedTypeInfo().GetTypeInfo())
                    return ConversionHelper::ToIxion(Objects::Object(lhs.GetTypeLibrary(), true));
                else
                    return ConversionHelper::ToIxion(lhs != rhs);
            default:
                throw std::runtime_error("Invalid operator");
        }
    }

    ixion::ref_ptr<value> ConversionHelper::Lookup(const Objects::Object& val, const std::string& identifier)
    {
        if (val.HasMember(identifier))
        {
            return new reflected_delegate(val, identifier);
        }
        auto converted = val.As<mofa::ref<jsValue>>();
        if (dynamic_cast<ObjectAsJsValue*>(converted.get()) == nullptr)
        {
            return Lookup(converted, identifier);
        }
        if (val.IsConvertibleTo<value*>())
            return val.As<value*>()->lookup(identifier);
        return nullptr;
    }

    ixion::ref_ptr<value> ConversionHelper::Lookup(const mofa::ref<jsValue>& val, const std::string& identifier)
    {
        if (!val)
            return nullptr;
        if(mofa::ref<jsValue> method = val->lookup(identifier))
        {
            DNVS::MoFa::Reflection::Objects::Object object(jsStack::stack()->GetTypeLibrary(), method);
            if (object.IsConvertibleTo<ref_ptr<jsValue_delegate, value>>())
                return object.As<ref_ptr<jsValue_delegate, value>>();
            else if(object.IsConvertibleTo<ref_ptr<reflected_delegate,value>>())
                return object.As<ref_ptr<reflected_delegate, value>>();
            return new jsValue_delegate(val, method);
        }
        DNVS::MoFa::Reflection::Objects::Object object(jsStack::stack()->GetTypeLibrary(), val);
        object = object.ConvertToDynamicType();
        DNVS::MoFa::Reflection::Objects::Object result(jsStack::stack()->GetTypeLibrary(), jsStack::stack()->GetConversionGraph()->TryUnwrapValue(object.GetVariant()));
        if (result.IsValid())
        {            
            if (result.HasMember(identifier))
                return new reflected_delegate(result, identifier, object);
        }
        return nullptr;
    }

    ixion::ref_ptr<value> ConversionHelper::Subscript(Objects::Object object, const value& index)
    {
        auto converted = object.As<mofa::ref<jsValue>>();
        if (dynamic_cast<ObjectAsJsValue*>(converted.get()) == nullptr)
        {
            return Subscript(converted, index);
        }
        auto objectIndex = Objects::Object(object.GetTypeLibrary(), const_cast<value&>(index).duplicate());
        return ConversionHelper::ToIxion(object[objectIndex.ConvertToDynamicType()]);
    }

    ixion::ref_ptr<value> ConversionHelper::Subscript(const mofa::ref<jsValue>& object, const value& index)
    {
        auto typeLibrary = jsStack::stack()->GetTypeLibrary();
        auto objectIndex = Objects::Object(typeLibrary, const_cast<value&>(index).duplicate()).ConvertToDynamicType();
        mofa::ref<jsValue> result = object->subscript(objectIndex.As<mofa::ref<jsValue>>());
        return ToIxion(result);
    }

}}

#pragma warning(pop)