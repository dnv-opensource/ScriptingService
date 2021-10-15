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
#include "jsValue_value.h"
#include "Reflection\Classes\Class.h"
#include "jsValue_delegate.h"
#include "value.Reflection.h"
#include "Reflection\Variants\VariantService.h"
#include "ConversionHelper.h"
#include "jsScript\jsStack.h"

#include "ref_ptr.Reflection.h"
#include "jsValue_iterator.h"
#include "ixlib_js_internals.hh"

#include "jsScript\jsAndOrOperators.h"
#include "jsScript\jsType.h"
#include "jsScript\jsConversions.h"


namespace ixion { namespace javascript {
    using namespace DNVS::MoFa::Reflection;
    jsValue_value::jsValue_value(const mofa::ref<jsValue>& object)
        : m_object(object)
    {
    }

    jsValue_value::~jsValue_value()
    {

    }

    value::value_type jsValue_value::getType() const
    {
        return VT_TYPE;
    }

    ref_ptr<value> jsValue_value::lookup(std::string const &identifier)
    {
        ref_ptr<value> result = ConversionHelper::Lookup(m_object, identifier);
        if (!result)
            return value::lookup(identifier);
        else
            return result;
    }

    const mofa::ref<jsValue>& jsValue_value::GetObject() const
    {
        return m_object;
    }

    ixion::ref_ptr<value> jsValue_value::eliminateWrappers()
    {
        return this;
    }

    ixion::ref_ptr<value> jsValue_value::duplicate()
    {
        return new jsValue_value(m_object);
    }

    ixion::ref_ptr<value> jsValue_value::subscript(value const &index)
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

    ixlib_iterator jsValue_value::begin() const
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

    ixlib_iterator jsValue_value::end() const
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

    std::size_t jsValue_value::size() const
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

    ref_ptr<value> jsValue_value::assign(ref_ptr<value> op2)
    {
        try {
            auto typeLibrary = jsStack::stack()->GetTypeLibrary();
            auto object = Objects::Object(typeLibrary, op2).ConvertToDynamicType();
            mofa::ref<jsValue> result = m_object->assign(nullptr, object.As<mofa::ref<jsValue>>());
            return ConversionHelper::ToIxion(result);
        }
        catch (...)
        {
            HandleExceptions();
            throw;
        }
    }

    ref_ptr<value> jsValue_value::operatorUnary(operator_id op) const
    {
        try {
            mofa::ref<jsValue> result;
            if (op == OP_UNARY_MINUS)
                result = -(*m_object);
            else if (op == OP_UNARY_PLUS)
                result = m_object;
            else if (op == OP_LOG_NOT)
                result = !(*m_object);
            if (result.get())
                return ConversionHelper::ToIxion(result);
        
            return value::operatorUnary(op);
        }
        catch (...)
        {
            HandleExceptions();
            throw;
        }
    }

    ref_ptr<value> jsValue_value::operatorBinary(operator_id op, ref_ptr<value> op2) const
    {
        try
        {
            mofa::ref<jsValue> outcome;
            auto typeLibrary = jsStack::stack()->GetTypeLibrary();
            auto object = Objects::Object(typeLibrary, op2).ConvertToDynamicType();
            mofa::ref<jsValue> rhs = object.As<mofa::ref<jsValue>>();
            switch (op)
            {
            case OP_PLUS:
                outcome = m_object->operator+(rhs);
                break;
            case OP_MINUS:
                outcome = m_object->operator-(rhs);
                break;
            case OP_MULTIPLY:
                outcome = m_object->operator*(rhs);
                break;
            case OP_DIVIDE:
                outcome = m_object->operator/(rhs); 
                break;
            case OP_EQUAL:
                outcome = m_object->operator==(rhs);
                break;
            case OP_NOT_EQUAL:
                outcome = m_object->operator!=(rhs);
                break;
            case OP_LESS_EQUAL:
                outcome = m_object->operator<=(rhs);
                break;
            case OP_GREATER_EQUAL:
                outcome = m_object->operator>=(rhs);
                break;
            case OP_LESS:
                outcome = m_object->operator<(rhs);
                break;
            case OP_GREATER:
                outcome = m_object->operator>(rhs); 
                break;
            }
            if (outcome)
                return ConversionHelper::ToIxion(outcome);
            return value::operatorBinary(op, op2);
        }
        catch (...)
        {
            HandleExceptions();
            throw;
        }
    }

    ixion::ref_ptr<value> jsValue_value::operatorBinaryShortcut(operator_id op, expression const& op2, context const& ctx) const
    {
        try {
            ref_ptr<value> result;
            auto typeLibrary = jsStack::stack()->GetTypeLibrary();
            Objects::Object lhs(typeLibrary, m_object);
            if (lhs.IsConvertibleTo<mofa::ref<jsAndOrOperators>>())
            {
                ref_ptr<value> result = op2.evaluate(ctx);
                Objects::Object rhs(typeLibrary, result);
                if (rhs.IsConvertibleTo<mofa::ref<jsAndOrOperators>>())
                {
                    if (op == OP_LOGICAL_OR)
                        return ConversionHelper::ToIxion(lhs.As<jsAndOrOperators&>() || rhs.As<jsAndOrOperators*>());
                    if (op == OP_LOGICAL_AND)
                        return ConversionHelper::ToIxion(lhs.As<jsAndOrOperators&>() && rhs.As<jsAndOrOperators*>());
                }
            }
            if (op == OP_LOGICAL_OR)
                return ConversionHelper::ToIxion(Objects::Object(typeLibrary, toBoolean() || op2.evaluate(ctx)->eliminateWrappers()->toBoolean()));
            if (op == OP_LOGICAL_AND)
                return ConversionHelper::ToIxion(Objects::Object(typeLibrary, toBoolean() && op2.evaluate(ctx)->eliminateWrappers()->toBoolean()));
            EXJS_THROWINFO_NO_LOCATION(ECJS_INVALID_OPERATION,
                (std::string(operator2string(op)) + std::string(" on ") + valueType2string(getType())).c_str())
        }
        catch (...)
        {
            HandleExceptions();
            throw;
        }
    }

    ixion::ref_ptr<value> jsValue_value::operatorBinaryModifying(operator_id op, ref_ptr<value> op2)
    {
        try
        {
            mofa::ref<jsValue> outcome;
            auto typeLibrary = jsStack::stack()->GetTypeLibrary();
            auto object = Objects::Object(typeLibrary, op2).ConvertToDynamicType();
            mofa::ref<jsValue> rhs = object.As<mofa::ref<jsValue>>();
            switch (op)
            {
                case OP_PLUS_ASSIGN:
                    outcome = m_object->operator+=(rhs);
                    break;
                case OP_MINUS_ASSIGN:
                    outcome = m_object->operator-=(rhs);
                    break;
                case OP_MUTLIPLY_ASSIGN:
                    outcome = m_object->operator*=(rhs);
                    break;
                case OP_DIVIDE_ASSIGN:
                    outcome = m_object->operator/=(rhs);
                    break;
            }
            return ConversionHelper::ToIxion(outcome);
            return value::operatorBinaryModifying(op, op2);
        }
        catch (...)
        {
            HandleExceptions();
            throw;
        }
    }

    std::string jsValue_value::toString() const
    {
        try {
            return fromJScript(m_object, jsType<std::string>());
        }
        catch (...)
        {
            HandleExceptions();
            throw;
        }
    }

    int jsValue_value::toInt() const
    {
        try {
            return fromJScript(m_object, jsType<int>());
        }
        catch (...)
        {
            HandleExceptions();
            throw;
        }
    }

    double jsValue_value::toFloat(bool allow_throw /*= false*/) const
    {
        try {
            return fromJScript(m_object, jsType<double>());
        }
        catch (...)
        {
            if (allow_throw)
            {
                HandleExceptions();
                throw;
            }
            else 
                return std::numeric_limits<double>::quiet_NaN();
        }
    }

    bool jsValue_value::toBoolean() const
    {
        try {
            return fromJScript(m_object, jsType<bool>());
        }
        catch (...)
        {
            HandleExceptions();
            throw;
        }        
    }

    std::string jsValue_value::GetErrorContext() const
    {
        std::string error = "";

        std::string name;
        if (m_object && m_object->getName(name))
        {
            error += "Identifier \'" + name + "\'";
        }
        error += ": ";
        return error;
    }

    class ConvertJsValue_valueToVariable : public TypeConversions::IConversion
    {
    public:
        virtual Variants::Variant Convert(const Variants::Variant& other)
        {
            jsValue_value* ref = static_cast<jsValue_value*>(other.Data());
            if (ref)
                return Variants::VariantService::ReflectType<mofa::ref<jsValue>>(ref->GetObject());
            else
                return other;
        }
        virtual void IntrusiveConvert(Variants::Variant& variable)
        {
            variable = Convert(variable);
        }
    };
    void DoReflect(TypeLibraryPointer typeLibrary, jsValue_value**)
    {
        using namespace DNVS::MoFa::Reflection::Classes;
        Class<jsValue_value, Public<value>, ref_ptr<jsValue_value, value>> cls(typeLibrary, "jsValue_value");
        typeLibrary->GetConversionGraph()->AddConversion(
            Types::TypeId<jsValue_value>(), Types::TypeId<void>(),
            TypeConversions::ConversionType::DynamicTypeConversion,
            std::make_shared<ConvertJsValue_valueToVariable>());
    }

}}