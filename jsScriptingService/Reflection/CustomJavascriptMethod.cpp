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
#include "CustomJavascriptMethod.h"
#include "Reflection/TypeLibraries/ITypeLibrary.h"
#include "Reflection/Objects/Object.h"
#include "jsScript/jsStack.h"
#include "jsScript/jsVTBL.h"
#include "jsScript/jsTypeLibrary.h"
#include "Reflection/Members/ArgumentInfo.h"
#include "Reflection/Attributes/TypeAttribute.h"
#include "Reflection/Attributes/AttributeCollectionService.h"
#include "ixlib_js_internals.hh"
#include "../Ixion/ConversionHelper.h"
#include "Reflection/Types/DynamicTypeTraits.h"

namespace DNVS { namespace MoFa { namespace Scripting {
    using namespace ixion::javascript;
    CustomJavascriptMethod::CustomJavascriptMethod(const std::string& name, ixion::ref_ptr<ixion::javascript::function> function, const std::vector<std::string>& ParameterTypeList, const std::string& ReturnType)
        : Reflection::Members::Method(name, jsStack::stack()->GetTypeLibrary()->GetConversionGraph())
        , m_function(function)
        , m_returnTypeAsString(ReturnType)
        , m_argumentTypes(ParameterTypeList)
    {
        std::vector<Reflection::Members::ArgumentInfoPointer> argumentInfo(GetArity());
        for (size_t i = 0; i < GetArity(); ++i)
            argumentInfo[i] = new Reflection::Members::ArgumentInfo(function->GetParameterNames().at(i), GetTypeInfo(ParameterTypeList.at(i)));
        SetArgumentList(argumentInfo, GetArity(), GetArity());
        m_returnType = GetTypeInfo(ReturnType);
    }

    CustomJavascriptMethod::~CustomJavascriptMethod()
    {

    }

    Reflection::Variants::Variant CustomJavascriptMethod::Invoke(const std::vector<Reflection::Variants::Variant>& arguments, Reflection::Members::MemberType type /*= MemberType::TypeAll*/)
    {
        value::parameter_list ixparams;
        for (size_t i = 0; i < arguments.size(); ++i)
            ixparams.push_back(ConversionHelper::ToIxion(Reflection::Objects::Object(jsStack::stack()->GetTypeLibrary(),arguments[i])));
        return Reflection::Objects::Object(jsStack::stack()->GetTypeLibrary(), m_function->call(ixparams)).ConvertToDynamicType().GetVariant();
    }

    Reflection::Variants::Variant CustomJavascriptMethod::InvokeNative(const std::vector<Reflection::Variants::Variant>& arguments)
    {
        return Invoke(arguments);
    }

    void CustomJavascriptMethod::Validate(const std::vector<Reflection::Variants::Variant>& arguments, Reflection::Members::MemberType type /*= MemberType::TypeAll*/)
    {
        return;
    }

    bool CustomJavascriptMethod::HasValidation() const
    {
        return false;
    }

    Reflection::Types::DecoratedTypeInfo CustomJavascriptMethod::GetReturnType() const
    {
        return m_returnType;
    }

    bool CustomJavascriptMethod::IsConst() const
    {
        return false;
    }

    size_t CustomJavascriptMethod::GetArity() const
    {
        return m_function->param_count();
    }

    Reflection::Members::MemberType CustomJavascriptMethod::GetMemberType() const
    {
        return Reflection::Members::MemberType::TypeStaticFunction;
    }

    std::string CustomJavascriptMethod::ToString(const std::vector<std::string>& arguments) const
    {
        std::string expression = GetName() + "(";
        bool isFirst = true;
        for (const std::string& arg : arguments)
        {
            if (isFirst)
                isFirst = false;
            else
                expression += ", ";
            expression += arg;
        }
        expression += ")";
        return expression;
    }

    void CustomJavascriptMethod::RegisterMemberDetails(const std::shared_ptr<Reflection::Members::MemberLoggerContext>& context)
    {
        
    }

    std::string CustomJavascriptMethod::Format(FormatType formatType)
    {
        if (formatType == IMember::FormatType::FunctionSignature)
            return FormatSignature();
        else if (formatType == IMember::FormatType::FunctionBody)
            return FormatBody();
        else
            return FormatSignature() + "\r\n{\r\n" + FormatBody() + "}";
    }

    std::string CustomJavascriptMethod::FormatSignature() const
    {
        std::string result = "function(";
        for (size_t i = 0; i < m_function->GetParameterNames().size(); ++i)
        {
            if (i != 0)
                result += ", ";
            result += m_function->GetParameterNames().at(i);
            if (!m_argumentTypes.at(i).empty())
                result += " : " + m_argumentTypes.at(i);
        }
        result += ")";
        if (!m_returnTypeAsString.empty())
            result += " : " + m_returnTypeAsString;
        return result;
    }

    std::string CustomJavascriptMethod::FormatBody() const
    {
        return m_function->toString();
    }

    Reflection::Types::DecoratedTypeInfo CustomJavascriptMethod::GetTypeInfo(const std::string& typeName)
    {
        if (typeName.empty())
            return Reflection::Types::TypeId<Reflection::Objects::Object>();
        if (_stricmp(typeName.c_str(), "number") == 0)
            return Reflection::Types::TypeId<double>();
        Reflection::Types::DecoratedTypeInfo oldTypeInfo;
        Reflection::Types::DecoratedTypeInfo newTypeInfo;
        if (jsVTBL* vtbl = jsStack::stack()->GetJsTypeLibrary().lookup(typeName))
        {
            auto type = Reflection::Types::DecoratedTypeInfo(vtbl->GetTypeInformation(), Reflection::Types::TypeDecoration::Pointer);
            oldTypeInfo = jsStack::stack()->GetTypeLibrary()->GetConversionGraph()->TryUnwrapType(type);
        }
        for (const auto& typePair : jsStack::stack()->GetTypeLibrary()->GetAllTypes())
        {
            if (_stricmp(typePair.second->GetName().c_str(), typeName.c_str()) == 0)
            {
                newTypeInfo = LookupType(typePair.second);
                if (newTypeInfo.IsValid())
                {
                    if (!Reflection::Types::IsPointer(newTypeInfo))
                        return newTypeInfo;
                }                    
            }
        }
        if (oldTypeInfo.IsValid())
            return oldTypeInfo;
        else
            return newTypeInfo;
    }
    Reflection::Types::DecoratedTypeInfo CustomJavascriptMethod::LookupType(const Reflection::TypeLibraries::TypePointer& type)
    {
        if (auto attribute = Reflection::Attributes::GetPointerToAttributeOrNull<Reflection::Attributes::TypeAttribute>(type))
            return attribute->GetDecoratedTypeInfo();
        for (const auto& memberPair : type->GetAllMembers())
        {
            if (memberPair.second->GetMemberType() == Reflection::Members::MemberType::TypeConstructor)
            {
                for (const auto& member : memberPair.second->GetOverloads())
                {
                    return member->GetReturnType();
                }
            }
        }
        return Reflection::Types::DecoratedTypeInfo(type->GetType(), 0);
    }

}}}

