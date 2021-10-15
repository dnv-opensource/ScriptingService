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
#include "IValueCreatorService.h"
#include "Reflection\TypeLibraries\TypeLibraryPointer.h"

namespace ixion { namespace javascript {
    using DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer;
    class ReflectionValueCreatorService : public IValueCreatorService
    {
    public:
        ReflectionValueCreatorService(TypeLibraryPointer typeLibraryPointer);
        ref_ptr<value> MakeUndefined() const override;
        ref_ptr<value> MakeNull() const override;
        ref_ptr<value> MakeConstant(signed long val) const override;
        ref_ptr<value> MakeConstant(bool val) const override;
        ref_ptr<value> MakeConstant(signed int val) const override;
        ref_ptr<value> MakeConstant(unsigned long val) const override;
        ref_ptr<value> MakeConstant(_w64 unsigned int val) const override;
        ref_ptr<value> MakeConstant(unsigned __int64 val) const override;
        ref_ptr<value> MakeConstant(double val) const override;
        ref_ptr<value> MakeConstant(std::string const &val) const override;
        ref_ptr<value> MakeLValue(ref_ptr<value> target) const override;
        ref_ptr<value> WrapConstant(ref_ptr<value> val) const override;
        ref_ptr<value> MakeConditional(ref_ptr<value> conditional, ref_ptr<expression> ifExpression, ref_ptr<expression> ifNotExpression, context const &ctx) const override;
        ref_ptr<value> MakeFunction(const std::string& name, const std::vector<std::string>& ParameterNameList, const std::vector<std::string>& ParameterTypeList, const std::string& ReturnType, ref_ptr<expression> Body, context const &ctx) const override;
    private:
        TypeLibraryPointer m_typeLibraryPointer;
    };
}}