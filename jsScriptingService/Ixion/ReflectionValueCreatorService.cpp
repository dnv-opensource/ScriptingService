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
#include "ReflectionValueCreatorService.h"
#include "reflected_value.h"
#include "jsScriptingService\Reflection\LValue.h"
#include "jsScriptingService\Reflection\Undefined.h"
#include "ref_ptr.Reflection.h"
#include "ixlib_js_internals.hh"
#include "Reflection/Attributes/ConditionalEvaluatorAttribute.h"
#include "Reflection\Attributes\AttributeCollection.h"
#include "..\Reflection\CustomJavascriptMethod.h"
#include "ConversionHelper.h"
#include "Reflection\Members\IMemberFwd.h"
#include "Scripting\INameService.h"
#include "jsScript\jsStack.h"
using namespace DNVS::MoFa::Reflection;
using namespace DNVS::MoFa::Scripting;
using namespace Attributes;
namespace ixion { namespace javascript {

    ReflectionValueCreatorService::ReflectionValueCreatorService(TypeLibraryPointer typeLibraryPointer)
        : m_typeLibraryPointer(typeLibraryPointer)
    {

    }

    ixion::ref_ptr<value> ReflectionValueCreatorService::MakeUndefined() const
    {
        return new reflected_value(Object(m_typeLibraryPointer, DNVS::MoFa::Scripting::Undefined()));
    }

    ixion::ref_ptr<value> ReflectionValueCreatorService::MakeNull() const
    {
        return new reflected_value(Object(m_typeLibraryPointer, nullptr));
    }

    ixion::ref_ptr<value> ReflectionValueCreatorService::MakeConstant(bool val) const
    {
        return new reflected_value(Object(m_typeLibraryPointer, val));
    }

    ixion::ref_ptr<value> ReflectionValueCreatorService::MakeConstant(signed long val) const
    {
        return new reflected_value(Object(m_typeLibraryPointer, val));
    }

    ixion::ref_ptr<value> ReflectionValueCreatorService::MakeConstant(signed int val) const
    {
        return new reflected_value(Object(m_typeLibraryPointer, val));
    }

    ixion::ref_ptr<value> ReflectionValueCreatorService::MakeConstant(unsigned long val) const
    {
        return new reflected_value(Object(m_typeLibraryPointer, val));
    }

    ixion::ref_ptr<value> ReflectionValueCreatorService::MakeConstant(_w64 unsigned int val) const
    {
        return new reflected_value(Object(m_typeLibraryPointer, val));
    }

    ixion::ref_ptr<value> ReflectionValueCreatorService::MakeConstant(unsigned __int64 val) const
    {
        return new reflected_value(Object(m_typeLibraryPointer, val));
    }

    ixion::ref_ptr<value> ReflectionValueCreatorService::MakeConstant(double val) const
    {
        return new reflected_value(Object(m_typeLibraryPointer, val));
    }

    ixion::ref_ptr<value> ReflectionValueCreatorService::MakeConstant(std::string const &val) const
    {
        return new reflected_value(Object(m_typeLibraryPointer, val));
    }

    ixion::ref_ptr<value> ReflectionValueCreatorService::MakeLValue(ref_ptr<value> target) const
    {
        using DNVS::MoFa::Reflection::Variants::VariantService;
        using DNVS::MoFa::Scripting::LValue;
        
        auto variant = VariantService::Reflect(std::make_unique<LValue>(Object(m_typeLibraryPointer, target).ConvertToDynamicType()));
        Object object(m_typeLibraryPointer, variant);
        return new reflected_value(object);
    }

    ixion::ref_ptr<value> ReflectionValueCreatorService::WrapConstant(ref_ptr<value> val) const
    {
        return val;
    }
    ixion::ref_ptr<value> TryParseExpression(ref_ptr<expression> expr, context const &ctx, bool& isReturn)
    {
        try {
            isReturn = false;
            if (!expr)
                return nullptr;
            else
                return expr->evaluate(ctx);
        }
        catch (ixion::javascript::return_exception r)
        {
            isReturn = true;
            return r.ReturnValue;
        }
    }

    class ConditionalException : public std::runtime_error
    {
    public:
        ConditionalException(ixion::javascript::code_location loc, std::exception& e)
            : std::runtime_error("[JS0002] Invalid operation (" + loc.stringify() + "): \n" + e.what())
        {}
    };
    ixion::ref_ptr<value> ReflectionValueCreatorService::MakeConditional(ref_ptr<value> conditional, ref_ptr<expression> ifExpression, ref_ptr<expression> ifNotExpression, context const &ctx) const
    {
        auto conditionalObject = Object(m_typeLibraryPointer, conditional).ConvertToDynamicType();
        auto type = conditionalObject.GetType();
        if (type && type->GetAttributeCollection().HasAttribute<ConditionalEvaluatorAttribute>())
        {
            try {
                const auto& conditionalEvaluator = type->GetAttributeCollection().GetAttribute<ConditionalEvaluatorAttribute>();
                bool ifReturn, ifNotReturn;
                ixion::ref_ptr<value> ifResult, ifNotResult;
                ifResult = TryParseExpression(ifExpression, ctx, ifReturn);
                ifNotResult = TryParseExpression(ifNotExpression, ctx, ifNotReturn);
                Object ifObject(m_typeLibraryPointer, ifResult), ifNotObject(m_typeLibraryPointer, ifNotResult);
                Object result = conditionalEvaluator.TryCreateConditional(conditionalObject, ifObject, ifReturn, ifNotObject, ifNotReturn);
                ref_ptr<value> returned = new reflected_value(Object(m_typeLibraryPointer, result));
                if (ifReturn && ifNotReturn)
                    throw ixion::javascript::return_exception(returned, ifExpression->getCodeLocation());
                else
                    return returned;
            }
            catch (ixion::javascript::return_exception)
            {
                throw;
            }
            catch (ConditionalException e)
            {
                throw;
            }
            catch(std::exception e)
            {
                throw ConditionalException(ifExpression->getCodeLocation(), e);
            }
        }
        throw;
    }
    
    ixion::ref_ptr<value> ReflectionValueCreatorService::MakeFunction(const std::string& name, const std::vector<std::string>& ParameterNameList, const std::vector<std::string>& ParameterTypeList, const std::string& ReturnType, ref_ptr<expression> Body, context const &ctx) const
    {
        auto fun = new function(std::make_shared<ReflectionValueCreatorService>(m_typeLibraryPointer), ParameterNameList, Body, ctx);
        DNVS::MoFa::Reflection::Members::MemberPointer method(new DNVS::MoFa::Scripting::CustomJavascriptMethod(name, fun, ParameterTypeList, ReturnType));
        DNVS::MoFa::Reflection::Objects::Object member(m_typeLibraryPointer, method);
        auto scope = jsStack::stack()->GetTypeLibrary()->GetServiceProvider().TryGetService<INewObjectScope>();
        if (scope)
            scope->AddNewObject(member);
        return ConversionHelper::ToIxion(member);
    }

}}

