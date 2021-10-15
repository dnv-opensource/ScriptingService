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
#include "Reflection/TypeLibraries/TypeLibraryPointer.h"
#include "jsScript/jsValue.h"

namespace ixion { namespace javascript {
    using DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer;
    class jsValue_value : public common_base_value
    {
    public:
        jsValue_value(const mofa::ref<jsValue>& object);
        ~jsValue_value();
        virtual value_type getType() const;
        virtual ref_ptr<value> lookup(std::string const &identifier) override;
        const mofa::ref<jsValue>& GetObject() const;
        virtual ref_ptr<value> eliminateWrappers();
        virtual ref_ptr<value> duplicate();
        virtual ref_ptr<value> subscript(value const &index);

        virtual ixlib_iterator begin() const;
        virtual ixlib_iterator end() const;
        virtual std::size_t size() const;

        ref_ptr<value> assign(ref_ptr<value> op2);
        ref_ptr<value> operatorUnary(operator_id op) const;
        ref_ptr<value> operatorBinary(operator_id op, ref_ptr<value> op2) const;
        ref_ptr<value> operatorBinaryShortcut(operator_id op, expression const& op2, context const& ctx) const;
        ref_ptr<value> operatorBinaryModifying(operator_id op, ref_ptr<value> op2);
        virtual std::string toString() const;
        virtual int toInt() const;
        virtual double toFloat(bool allow_throw = false) const;
        virtual bool toBoolean() const;
    private:
        mofa::ref<jsValue> m_object;
        virtual std::string GetErrorContext() const override;
    };
    void DoReflect(TypeLibraryPointer typeLibrary, jsValue_value**);
}}