#pragma once
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
#include "ixlib_javascript.hh"
#include "../Ixion/reflected_value.h"
#include "../Reflection/LValue.h"
#include "Operators/Invoker.h"
#include <type_traits>
#include "Reflection/Types/DynamicTypeTraits.h"
namespace ixion { namespace javascript {
    using namespace DNVS::MoFa::Operators;
    using namespace DNVS::MoFa::Operators::Tags;
    using namespace DNVS::MoFa::Reflection::Objects;
    using DNVS::MoFa::Scripting::LValue;
    using DNVS::MoFa::Reflection::Variants::Variant;
    using namespace DNVS::MoFa::Reflection::Types;
    template<typename Callback, typename Fallback>
    auto TryForIntegralTypes(const DecoratedTypeInfo& decoratedType, Callback& callback, Fallback& fallback)
    {
        if (decoratedType.GetDecoration() != 0)
            return fallback();
        std::type_index type = decoratedType.GetTypeInfo();
        if (type == typeid(char))
            return callback(char());
        if (type == typeid(signed char))
            return callback(signed char());
        if (type == typeid(short))
            return callback(short());
        if (type == typeid(int))
            return callback(int());
        if (type == typeid(long))
            return callback(long());
        if (sizeof(size_t) >= 8 && type == typeid(long long))
            return callback(long long());
        if (type == typeid(unsigned char))
            return callback(unsigned char());
        if (type == typeid(unsigned short))
            return callback(unsigned short());
        if (type == typeid(unsigned int))
            return callback(unsigned int());
        if (type == typeid(unsigned long))
            return callback(unsigned long());
        if (sizeof(size_t) >= 8 && type == typeid(unsigned long long))
            return callback(unsigned long long());
        return fallback();
    }

    template<typename Callback, typename Fallback>
    auto TryForOptimizedTypes(const DecoratedTypeInfo& decoratedType, Callback& callback, Fallback& fallback)
    {
        if (decoratedType.GetDecoration() != 0)
            return fallback();
        std::type_index type = decoratedType.GetTypeInfo();
        if (sizeof(size_t) >= 8 && type == typeid(double))
            return callback(double());
        if (type == typeid(float))
            return callback(float());
        if (type == typeid(bool))
            return callback(bool());
        return TryForIntegralTypes(decoratedType, callback, fallback);
    }

    template<typename T, typename Tag>
    struct UnaryModifyingOperatorOptimizer
    {
        static ref_ptr<value> Invoke(const ref_ptr<reflected_value, value>& value, LValue& lvalue)
        {
            T t = T();
            using ReturnType = decltype(Invoker<Tag>::Invoke(t));
            bool returnCopy = !std::is_reference_v<ReturnType>;
            if (returnCopy)
            {
                LValue copy = lvalue;
                InvokeImpl(lvalue);
                return new reflected_value(Object(copy.GetObject().GetTypeLibrary(), copy));
            }
            else
            {
                InvokeImpl(lvalue);
                return value;
            }
        }
        static void InvokeImpl(LValue& lvalue)
        {
            Variants::Variant v = lvalue.GetObject().GetVariant();
            union {
                void* ptr;
                T value;
            } u;
            u.ptr = v.GetData();
            Invoker<Tag>::Invoke(u.value);
            v.SetData(reinterpret_cast<void*>(u.ptr));
            const_cast<Object&>(lvalue.GetObject()).Reset(v);
        }
    };

    template<typename Tag>
    ref_ptr<value> TryOptimizedUnaryModifyingOperator(const ref_ptr<reflected_value, value>& value, Object& object)
    {
        auto fallback = [&]() {
            return ConversionHelper::ToIxion(Invoker<Tag>::Invoke(object));
        };
        if (value->GetObject().GetDecoratedTypeInfo().GetTypeInfo() == typeid(LValue))
        {
            LValue& lvalue = *static_cast<LValue*>(object.GetVariant().GetData());
            return TryForIntegralTypes(lvalue.GetObject().GetDecoratedTypeInfo(), [&](auto a) { return UnaryModifyingOperatorOptimizer<decltype(a), Tag>::Invoke(value, lvalue); }, fallback);
        }
        else
            return fallback();
    }


    inline Variant TryUnwrapValue(const Object& subject)
    {
        if (subject.GetDecoratedTypeInfo().GetTypeInfo() == typeid(LValue))
        {
            LValue* lvalue = static_cast<LValue*>(subject.GetVariant().GetData());
            return lvalue->GetObject().GetVariant();
        }
        else
            return subject.GetVariant();
    }

    
    template<typename Tag, typename T1, typename T2>
    ref_ptr<value> TryInvokeBinaryOperator(const TypeLibraryPointer& typeLibrary, Tag, T1 t1, T2 t2)
    {
        return new reflected_value(Objects::Object(typeLibrary, Invoker<Tag>::Invoke(t1, t2)));
    }
    template<typename T1, typename T2>
    ref_ptr<value> TryInvokeBinaryOperator(const TypeLibraryPointer& typeLibrary, Divides, T1 t1, T2 t2)
    {
        return new reflected_value(Objects::Object(typeLibrary, Invoker<Divides>::Invoke(t1, (double)t2)));
    }
    template<typename T1, typename T2>
    std::enable_if_t<std::is_integral_v<T1> && !std::is_same_v<T1, bool> && 
                     std::is_integral_v<T2> && !std::is_same_v<T2, bool>,ref_ptr<value>> TryInvokeBinaryOperator(const TypeLibraryPointer& typeLibrary, Minus, T1 t1, T2 t2)
    {
        return new reflected_value(Objects::Object(typeLibrary, Invoker<Minus>::Invoke(std::make_signed_t<T1>(t1), std::make_signed_t<T2>(t2))));
    }
    template<typename Tag, typename T>
    struct OptimizedBinaryOperatorSecondArgument
    {
        OptimizedBinaryOperatorSecondArgument(T v1, const Variant& v2, const TypeLibraryPointer& typeLibrary)
            : m_v1(v1)
            , m_v2(v2)
            , m_typeLibrary(typeLibrary)
        {}
        template<typename U>
        ref_ptr<value> operator()(U v)
        {
            union {
                void* ptr;
                U data;
            };
            ptr = m_v2.GetData();
            return TryInvokeBinaryOperator(m_typeLibrary, Tag(),m_v1, data);
        }
        T m_v1;
        const Variant& m_v2;
        TypeLibraryPointer m_typeLibrary;
    };

    template<typename Tag>
    struct OptimizedBinaryOperatorFirstArgument
    {
        OptimizedBinaryOperatorFirstArgument(const Variant& v1, const Variant& v2, const TypeLibraryPointer& typeLibrary)
            : m_v1(v1)
            , m_v2(v2)
            , m_typeLibrary(typeLibrary)
        {}
        template<typename T>
        ref_ptr<value> operator()(T v)
        {
            union {
                void* ptr;
                T data;
            };
            ptr = m_v1.GetData();
            return TryForOptimizedTypes(m_v2.GetDecoratedTypeInfo(), OptimizedBinaryOperatorSecondArgument<Tag, T>(data, m_v2, m_typeLibrary), []() {return ref_ptr<value>(); });
        }
        const Variant& m_v1;
        const Variant& m_v2;
        TypeLibraryPointer m_typeLibrary;
    };
    template<typename Tag>
    ref_ptr<value> TryOptimizedBinaryOperator(const Object& lhs, const Object& rhs)
    {
        Variant v1 = TryUnwrapValue(lhs);
        Variant v2 = TryUnwrapValue(rhs);
        return TryForOptimizedTypes(v1.GetDecoratedTypeInfo(), OptimizedBinaryOperatorFirstArgument<Tag>(v1, v2, lhs.GetTypeLibrary()), []() {return ref_ptr<value>(); });
    }
}}