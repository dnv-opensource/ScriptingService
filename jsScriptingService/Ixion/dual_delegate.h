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

#include "common_base_value.h"
#include "mofaTools\ref.h"
#include "jsScript\jsValue.h"
#include "Reflection\TypeLibraries\TypeLibraryPointer.h"

namespace ixion { namespace javascript {
    using DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer;
    class dual_delegate : public value
    {
    public:
        dual_delegate(const ref_ptr<value>& delegate1, const ref_ptr<value>& delegate2);
        virtual ref_ptr<value> eliminateWrappers() override;
        virtual ref_ptr<value> duplicate() override;
        virtual ref_ptr<value> duplicateAndResolveDelegate() override;
        virtual value_type getType() const;
        virtual ref_ptr<value> lookup(std::string const &identifier) override;
        virtual ref_ptr<value> assign(ref_ptr<value> op2) override;
        virtual ref_ptr<value> call(parameter_list const &parameters) override;
        virtual ref_ptr<value> construct(parameter_list const& parameters) override;
        virtual ref_ptr<value> operatorBinary(operator_id op, ref_ptr<value> op2) const override;
        virtual ref_ptr<value> operatorUnary(operator_id op) const override;
        virtual ref_ptr<value> operatorBinaryShortcut(operator_id op, expression const &op2, context const &ctx) const override;
        virtual ref_ptr<value> operatorUnaryModifying(operator_id op) override;
        virtual ref_ptr<value> operatorBinaryModifying(operator_id op, ref_ptr<value> op2) override;
        virtual ref_ptr<value> subscript(value const &index);

        virtual ixlib_iterator begin() const;
        virtual ixlib_iterator end() const;
        virtual std::size_t size() const;
        virtual std::string toString() const;
        virtual int toInt() const;
        virtual double toFloat(bool allow_throw = false) const;
        virtual bool toBoolean() const;
    private:
        ref_ptr<value> m_delegate1; 
        ref_ptr<value> m_delegate2;
    };
    void DoReflect(TypeLibraryPointer typeLibrary, dual_delegate**);
}}