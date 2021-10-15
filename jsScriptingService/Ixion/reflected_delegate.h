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
    using DNVS::MoFa::Reflection::Objects::Delegate;
    using DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer;

    class reflected_delegate : public common_base_value
    {
    public:
        reflected_delegate(const Object& object, const std::string& name, const Object& jsObject = Object());
        virtual ~reflected_delegate();
        virtual value_type getType() const;
        //If the delegate contains a property getter, it will be called here, and an object of type ReflectedObjectValue will be returned.
        virtual ixlib_iterator begin() const;
        virtual ixlib_iterator end() const;
        virtual std::size_t size() const;

        virtual ref_ptr<value> eliminateWrappers();
        virtual ref_ptr<value> duplicate();
        virtual ref_ptr<value> duplicateAndResolveDelegate();
        virtual ref_ptr<value> operatorBinary(operator_id op, ref_ptr<value> op2) const;
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
        //If this delegate contains a property setter, it will be called here.
        //(In reality, the implementation is equivalent of call with one argument.
        virtual ref_ptr<value> assign(ref_ptr<value> op2);
        //Calls the method with the given parameters. All the parameters will be wrapped as variants before passed to the function.
        virtual ref_ptr<value> call(parameter_list const& parameters);
        virtual ref_ptr<value> construct(parameter_list const& parameters);
        Delegate GetDelegate() const;
        const Object& GetObject() const { return m_object; }
        const std::string& GetName() const { return m_name; }

        virtual std::string toString() const;
        virtual int toInt() const;
        virtual double toFloat(bool allow_throw = false) const;
        virtual bool toBoolean() const;
    private:
        virtual std::string GetErrorContext() const override;
        ref_ptr<value> CreateObject() const;
        Object m_object;
        Object m_jsObject;
        std::string m_name;
    };
    void DoReflect(TypeLibraryPointer typeLibrary, reflected_delegate**);
}}