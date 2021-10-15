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
#include "Reflection/Members/Method.h"
#include "Reflection/TypeLibraries/TypeLibraryPointer.h"

#include "ixlib_garbage.hh"
#include <vector>
#include "Reflection/TypeLibraries/IType.h"

namespace ixion {namespace javascript {
    class value;
    class function;
    struct context;
}}

namespace DNVS { namespace MoFa { namespace Scripting {
    class CustomJavascriptMethod : public DNVS::MoFa::Reflection::Members::Method
    {
    public:
        CustomJavascriptMethod(const std::string& name, ixion::ref_ptr<ixion::javascript::function> function, const std::vector<std::string>& ParameterTypeList, const std::string& ReturnType);
        ~CustomJavascriptMethod();
        Reflection::Variants::Variant Invoke(const std::vector<Reflection::Variants::Variant>& arguments, Reflection::Members::MemberType type = Reflection::Members::MemberType::TypeAll) override;
        Reflection::Variants::Variant InvokeNative(const std::vector<Reflection::Variants::Variant>& arguments) override;
        void Validate(const std::vector<Reflection::Variants::Variant>& arguments, Reflection::Members::MemberType type = Reflection::Members::MemberType::TypeAll) override;
        bool HasValidation() const override;
        Reflection::Types::DecoratedTypeInfo GetReturnType() const override;
        bool IsConst() const override;
        size_t GetArity() const override;
        Reflection::Members::MemberType GetMemberType() const override;
        std::string ToString(const std::vector<std::string>& arguments) const override;
        void RegisterMemberDetails(const std::shared_ptr<Reflection::Members::MemberLoggerContext>& context) override;
        std::string Format(FormatType formatType) override;
    private:
        std::string FormatSignature() const;
        std::string FormatBody() const;
        ixion::ref_ptr<ixion::javascript::function> m_function;
        Reflection::Types::DecoratedTypeInfo m_returnType;
        std::vector<std::string> m_argumentTypes;
        std::string m_returnTypeAsString;
        static Reflection::Types::DecoratedTypeInfo GetTypeInfo(const std::string& typeName);
        static Reflection::Types::DecoratedTypeInfo LookupType(const Reflection::TypeLibraries::TypePointer& type);
    };
}}}