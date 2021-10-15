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
#include "Reflection\Objects\Object.h"

namespace ixion { namespace javascript {
    using DNVS::MoFa::Reflection::Objects::Object;
    using DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer;
    class reflected_value : public common_base_value
    {
    public:
        reflected_value(const Object& object);
        ~reflected_value();
        virtual value_type getType() const;
        virtual ref_ptr<value> eliminateWrappers();
        virtual ref_ptr<value> duplicate();

        virtual ixlib_iterator begin() const;
        virtual ixlib_iterator end() const;
        virtual std::size_t size() const;

        //Invokes a binary operator on the m_object. op2 is wrapped as a variant before invocation.
        virtual ref_ptr<value> operatorBinary(operator_id op, ref_ptr<value> op2) const;
        Object GetObject() const;

        ref_ptr<value> assign(ref_ptr<value> op2);
        //Invokes a unary operator on the m_object. op2 is wrapped as a variant before invocation.
        virtual ref_ptr<value> operatorUnary(operator_id op) const;
        //Forwards to base implementation. Not implemented here.
        virtual ref_ptr<value> operatorBinaryShortcut(operator_id op, expression const &op2,
            context const &ctx) const;
        //Forwards to m_object modifying unary operators, ++m_object, --m_object etc.
        virtual ref_ptr<value> operatorUnaryModifying(operator_id op);
        //Forwards to m_object modifying binary operators, m_object += op2 etc.
        virtual ref_ptr<value> operatorBinaryModifying(operator_id op, ref_ptr<value> op2);
        virtual ref_ptr<value> lookup(std::string const &identifier);
        ref_ptr<value> subscript(value const &index);
        virtual ref_ptr<value> call(parameter_list const &parameters);

        //Uses type conversion to convert m_object to string
        virtual std::string toString() const;
        //Uses type conversion to convert m_object to int
        virtual int toInt() const;
        //Uses type conversion to convert m_object to double
        virtual double toFloat(bool allow_throw = false) const;
        //Uses type conversion to convert m_object to bool
        virtual bool toBoolean() const;
    private:
        virtual std::string GetErrorContext() const override;
        Object m_object;
    };
    void DoReflect(TypeLibraryPointer typeLibrary, reflected_value**);
}}