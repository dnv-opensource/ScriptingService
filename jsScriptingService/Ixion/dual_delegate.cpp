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
#include "dual_delegate.h"
#include "ixlib_iterator.hpp"
#include "Reflection/Classes/Class.h"
#include "EliminateWrappersConversion.h"
#include "value.Reflection.h"

namespace ixion { namespace javascript {

    dual_delegate::dual_delegate(const ref_ptr<value>& delegate1, const ref_ptr<value>& delegate2)
        : m_delegate1(delegate1)
        , m_delegate2(delegate2)
    {}

    ref_ptr<value> dual_delegate::eliminateWrappers()
    {
        try {
            ref_ptr<value> result = m_delegate1->eliminateWrappers();
            if (result != m_delegate1)
                return result;
        }
        catch(...) {}
        return m_delegate2->eliminateWrappers();
    }

    ref_ptr<value> dual_delegate::duplicate()
    {
        return new dual_delegate(m_delegate1, m_delegate2);
    }

    ref_ptr<value> dual_delegate::duplicateAndResolveDelegate()
    {
        return eliminateWrappers();
    }

    value::value_type dual_delegate::getType() const
    {
        return value::VT_TYPE;
    }

    ref_ptr<value> dual_delegate::lookup(std::string const &identifier)
    {
        try {
            return m_delegate1->lookup(identifier);
        }
        catch (...) {
            return m_delegate2->lookup(identifier);
        }
    }

    ixion::ref_ptr<value> dual_delegate::assign(ref_ptr<value> op2)
    {
        try {
            return m_delegate1->assign(op2);
        }
        catch(...) {
            return m_delegate2->assign(op2);
        }
    }

    ixion::ref_ptr<value> dual_delegate::call(parameter_list const &parameters)
    {
        try {
            return m_delegate1->call(parameters);
        }
        catch (...)
        {
            return m_delegate2->call(parameters);
        }
    }

    ixion::ref_ptr<value> dual_delegate::construct(parameter_list const& parameters)
    {
        try {
            return m_delegate1->construct(parameters);
        }
        catch (...)
        {
            return m_delegate2->construct(parameters);
        }
    }

    ixion::ref_ptr<value> dual_delegate::operatorBinary(operator_id op, ref_ptr<value> op2) const
    {
        try {
            return m_delegate1->operatorBinary(op, op2);
        }
        catch (...)
        {
            return m_delegate2->operatorBinary(op, op2);
        }
    }

    ixion::ref_ptr<value> dual_delegate::operatorUnary(operator_id op) const
    {
        try {
            return m_delegate1->operatorUnary(op);
        }
        catch (...)
        {
            return m_delegate2->operatorUnary(op);
        }
    }

    ixion::ref_ptr<value> dual_delegate::operatorBinaryShortcut(operator_id op, expression const &op2, context const &ctx) const
    {
        try {
            return m_delegate1->operatorBinaryShortcut(op, op2, ctx);
        }
        catch (...)
        {
            return m_delegate2->operatorBinaryShortcut(op, op2, ctx);
        }
    }

    ixion::ref_ptr<value> dual_delegate::operatorUnaryModifying(operator_id op)
    {
        try {
            return m_delegate1->operatorUnaryModifying(op);
        }
        catch (...)
        {
            return m_delegate2->operatorUnaryModifying(op);
        }
    }

    ixion::ref_ptr<value> dual_delegate::operatorBinaryModifying(operator_id op, ref_ptr<value> op2)
    {
        try {
            return m_delegate1->operatorBinaryModifying(op, op2);
        }
        catch (...)
        {
            return m_delegate2->operatorBinaryModifying(op, op2);
        }
    }

    ixion::ref_ptr<value> dual_delegate::subscript(value const &index)
    {
        try {
            return m_delegate1->subscript(index);
        }
        catch (...)
        {
            return m_delegate2->subscript(index);
        }

    }

    ixion::javascript::ixlib_iterator dual_delegate::begin() const
    {
        try {
            return m_delegate1->begin();
        }
        catch (...)
        {
            return m_delegate2->begin();
        }
    }

    ixion::javascript::ixlib_iterator dual_delegate::end() const
    {
        try {
            return m_delegate1->end();
        }
        catch (...)
        {
            return m_delegate2->end();
        }
    }

    std::size_t dual_delegate::size() const
    {
        try {
            return m_delegate1->size();
        }
        catch (...)
        {
            return m_delegate2->size();
        }

    }

    std::string dual_delegate::toString() const
    {
        try {
            return m_delegate1->toString();
        }
        catch (...)
        {
            return m_delegate2->toString();
        }
    }

    int dual_delegate::toInt() const
    {
        try {
            return m_delegate1->toInt();
        }
        catch (...)
        {
            return m_delegate2->toInt();
        }
    }

    double dual_delegate::toFloat(bool allow_throw /*= false*/) const
    {
        try {
            return m_delegate1->toFloat(allow_throw);
        }
        catch (...)
        {
            return m_delegate2->toFloat(allow_throw);
        }
    }

    bool dual_delegate::toBoolean() const
    {
        try {
            return m_delegate1->toBoolean();
        }
        catch (...)
        {
            return m_delegate2->toBoolean();
        }
    }

    void DoReflect(TypeLibraryPointer typeLibrary, dual_delegate**)
    {        
        using namespace DNVS::MoFa::Reflection;
        using namespace Classes;
        Class<dual_delegate, Public<value>, ref_ptr<dual_delegate, value>> cls(typeLibrary, "dual_delegate");
        typeLibrary->GetConversionGraph()->AddConversion(
            Types::TypeId<dual_delegate>(), Types::TypeId<void>(),
            TypeConversions::ConversionType::DynamicTypeConversion,
            std::make_shared<EliminateWrappersConversion>());
    }

}}

