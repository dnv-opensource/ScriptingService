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
#include "reflected_value.h"
#include "reflected_iterator.h"
#include "reflected_delegate.h"
#include "Reflection\Classes\Class.h"
#include "value.Reflection.h"
#include "ConversionHelper.h"
#include "ref_ptr.Reflection.h"
#include "ixlib_js_internals.hh"
#include "Scripting/INameService.h"
#include "OptimizedOperatorHandlers.h"
#include "Reflection\Objects\InvokeBinaryOperator.h"

namespace ixion { namespace javascript {
    using namespace DNVS::MoFa::Reflection;
    using namespace DNVS::MoFa::Scripting;
    using Variants::VariantService;

    reflected_value::reflected_value(const Object& object)
        : m_object(object)
    {

    }

    reflected_value::~reflected_value()
    {

    }

    value::value_type reflected_value::getType() const
    {
        return VT_TYPE;
    }

    ref_ptr<value> reflected_value::eliminateWrappers()
    {        
        if (m_object.GetDecoratedTypeInfo().GetTypeInfo() == typeid(LValue))
            return ConversionHelper::ToIxion(m_object.ConvertToDynamicType());
        else
            return this;
    }

    ref_ptr<value> reflected_value::duplicate()
    {
        return new reflected_value(m_object);
    }

    ixlib_iterator reflected_value::begin() const
    {
        try {
            return ConversionHelper::Begin(m_object);
        }
        catch (...)
        {
            HandleExceptions();
            throw;
        }
    }

    ixlib_iterator reflected_value::end() const
    {
        try {
            return ConversionHelper::End(m_object);
        }
        catch (...)
        {
            HandleExceptions();
            throw;
        }
    }

    std::size_t reflected_value::size() const
    {
        try {
            return ConversionHelper::Size(m_object);
        }
        catch (...)
        {
            HandleExceptions();
            throw;
        }
    }


    ref_ptr<value> reflected_value::assign(ref_ptr<value> op2)
    {
        try {
            auto other = Object(m_object.GetTypeLibrary(), op2).ConvertToDynamicType();
            m_object.Reset(Objects::InvokeBinaryModifyingOperator<DNVS::MoFa::Operators::Tags::Assign>(m_object.GetTypeLibrary(), m_object.GetVariant(), other.GetVariant()));

            return ConversionHelper::ToIxion(Object(m_object.GetTypeLibrary(), m_object.Assign(other)));
        }
        catch (...)
        {
            HandleExceptions();
            throw;
        }
    }

    ref_ptr<value> reflected_value::operatorBinary(operator_id op, ref_ptr<value> op2) const
    {
        try {
            auto other = Object(m_object.GetTypeLibrary(), op2).ConvertToDynamicType();
            return ConversionHelper::InvokeBinaryOperator(m_object, other, op);
        }
        catch (...)
        {
            HandleExceptions();
            throw;
        }
    }

    Object reflected_value::GetObject() const
    {
        return m_object;
    }

    ref_ptr<value> reflected_value::operatorUnary(operator_id op) const
    {
        try {
            switch (op)
            {
                case OP_UNARY_PLUS:
                    return ConversionHelper::ToIxion(+m_object);
                case OP_UNARY_MINUS:
                    return ConversionHelper::ToIxion(-m_object);
                case OP_LOG_NOT:
                    return ConversionHelper::ToIxion(!m_object);
                case OP_BIN_NOT:
                    return ConversionHelper::ToIxion(~m_object);
                default:
                    return value::operatorUnary(op);
            }
        }
        catch (...)
        {
            HandleExceptions();
            throw;
        }
    }

    ref_ptr<value> reflected_value::operatorBinaryShortcut(operator_id op, expression const &op2, context const &ctx) const
    {
        try {
            if (op == OP_LOGICAL_OR)
                return ConversionHelper::ToIxion(Objects::Object(m_object.GetTypeLibrary(), toBoolean() || op2.evaluate(ctx)->eliminateWrappers()->toBoolean()));
            if (op == OP_LOGICAL_AND)
                return ConversionHelper::ToIxion(Objects::Object(m_object.GetTypeLibrary(), toBoolean() && op2.evaluate(ctx)->eliminateWrappers()->toBoolean()));
            EXJS_THROWINFO_NO_LOCATION(ECJS_INVALID_OPERATION,
                (std::string(operator2string(op)) + std::string(" on ") + valueType2string(getType())).c_str())            
        }
        catch (...)
        {
            HandleExceptions();
            throw;
        }
    }

    ref_ptr<value> reflected_value::operatorUnaryModifying(operator_id op)
    {
        try {
            switch (op)
            {
                case OP_PRE_INCREMENT:
                    return TryOptimizedUnaryModifyingOperator<DNVS::MoFa::Operators::Tags::PreInc>(this, m_object);
                case OP_POST_INCREMENT:
                    return TryOptimizedUnaryModifyingOperator<DNVS::MoFa::Operators::Tags::PostInc>(this, m_object);
                case OP_PRE_DECREMENT:
                    return TryOptimizedUnaryModifyingOperator<DNVS::MoFa::Operators::Tags::PreDec>(this, m_object);
                case OP_POST_DECREMENT:
                    return TryOptimizedUnaryModifyingOperator<DNVS::MoFa::Operators::Tags::PostDec>(this, m_object);
                default:
                    return value::operatorUnaryModifying(op);
            }
        }
        catch (...)
        {
            HandleExceptions();
            throw;
        }

    }

    ref_ptr<value> reflected_value::operatorBinaryModifying(operator_id op, ref_ptr<value> op2)
    {
        try {
            auto other = Object(m_object.GetTypeLibrary(), op2).ConvertToDynamicType();

            switch (op)
            {
                case OP_PLUS_ASSIGN:
                    return ConversionHelper::ToIxion(m_object += other);
                case OP_MINUS_ASSIGN:
                    return ConversionHelper::ToIxion(m_object -= other);
                case OP_MUTLIPLY_ASSIGN:
                    return ConversionHelper::ToIxion(m_object *= other);
                case OP_DIVIDE_ASSIGN:
                    return ConversionHelper::ToIxion(m_object /= other);
                case OP_BIT_XOR_ASSIGN:
                    return ConversionHelper::ToIxion(m_object ^= other);
                case OP_MODULO_ASSIGN:
                    return ConversionHelper::ToIxion(m_object %= other);
                case OP_BIT_AND_ASSIGN:
                    return ConversionHelper::ToIxion(m_object &= other);
                case OP_BIT_OR_ASSIGN:
                    return ConversionHelper::ToIxion(m_object |= other);
                case OP_LEFT_SHIFT_ASSIGN:
                    return ConversionHelper::ToIxion(m_object <<= other);
                case OP_RIGHT_SHIFT_ASSIGN:
                    return ConversionHelper::ToIxion(m_object >>= other);
                default:
                    return value::operatorBinaryModifying(op, op2);
            }
        }
        catch (...)
        {
            HandleExceptions();
            throw;
        }
    }

    ref_ptr<value> reflected_value::lookup(std::string const &identifier)
    {        
        auto result = ConversionHelper::Lookup(m_object, identifier);
        if (!result)
            return value::lookup(identifier);
        else
            return result;
    }

    ixion::ref_ptr<value> reflected_value::subscript(value const &index)
    {
        try {
            return ConversionHelper::Subscript(m_object, index);
        }
        catch (...)
        {
            HandleExceptions();
            throw;
        }
    }

    ixion::ref_ptr<value> reflected_value::call(parameter_list const &parameters)
    {
        try {
            if (m_object.IsConvertibleTo<ref_ptr<callable_with_parameters, value>>())
            {
                return m_object.As<ref_ptr<callable_with_parameters, value>>()->call(parameters);
            }
            else if (m_object.IsConvertibleTo<Members::MemberPointer>())
            {
                std::vector<Variant> args(parameters.size());
                for (size_t i = 0; i < parameters.size(); ++i)
                    args[i] = Object(m_object.GetTypeLibrary(), parameters[i]).ConvertToDynamicType().GetVariant();
                return ConversionHelper::ToIxion(Object(m_object.GetTypeLibrary(), m_object.As<Members::MemberPointer>()->Invoke(args)));
            }
            else
                return value::call(parameters);
        }
        catch (...)
        {
            HandleExceptions();
            throw;
        }
    }

    std::string reflected_value::toString() const
    {
        try {
            return m_object.As<std::string>();
        }
        catch (...)
        {
            HandleExceptions();
            throw;
        }        
    }

    int reflected_value::toInt() const
    {
        try {
            return m_object.As<int>();
        }
        catch (...)
        {
            HandleExceptions();
            throw;
        }
    }

    double reflected_value::toFloat(bool allow_throw /*= false*/) const
    {
        try {
            if (allow_throw || m_object.IsConvertibleTo<double>())
                return m_object.As<double>();

            return std::numeric_limits<double>::quiet_NaN();
        }
        catch (...)
        {
            HandleExceptions();
            throw;
        }
    }

    bool reflected_value::toBoolean() const
    {
        try {
            return m_object.As<bool>();
        }
        catch (...)
        {
            HandleExceptions();
            throw;
        }
    }

    std::string reflected_value::GetErrorContext() const
    {
        std::string error = "";
        if (m_object.GetTypeLibrary())
        {
            auto service = m_object.GetTypeLibrary()->GetServiceProvider().TryGetService<INameService>();
            if (service && service->HasName(m_object))
                error += "Identifier '" + service->GetName(m_object) + "'";
        }        
        error += ": ";
        return error;
    }

    class ConvertJsReflectedValueToVariable : public TypeConversions::IConversion
    {
    public:
        virtual Variants::Variant Convert(const Variants::Variant& other)
        {
            reflected_value* ref = static_cast<reflected_value*>(other.Data());
            if (ref)
                return ref->GetObject().GetVariant();
            return other;
        }
        virtual void IntrusiveConvert(Variants::Variant& variable)
        {
            variable = Convert(variable);
        }
    };

    void DoReflect(TypeLibraryPointer typeLibrary, reflected_value**)
    {
        using namespace Classes;
        Class<reflected_value, Public<value>, ref_ptr<reflected_value, value>> cls(typeLibrary, "reflected_value");
        typeLibrary->GetConversionGraph()->AddConversion(
            Types::TypeId<reflected_value>(), Types::TypeId<void>(),
            TypeConversions::ConversionType::DynamicTypeConversion,
            std::make_shared<ConvertJsReflectedValueToVariable>());
    }

}}

