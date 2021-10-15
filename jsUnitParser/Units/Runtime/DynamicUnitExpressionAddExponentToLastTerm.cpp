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
#include "DynamicUnitExpressionAddExponentToLastTerm.h"
#include "boost\lexical_cast.hpp"

namespace DNVS {namespace MoFa { namespace Units { namespace Parser {

    DynamicUnitExpressionAddExponentToLastTerm::DynamicUnitExpressionAddExponentToLastTerm(IUnitHelper& helper, ExpressionPointer unitExpression,
        ExpressionPointer exponent,
        CodeLocation const &loc)
        : DynamicUnitExpression(helper, loc), m_unitExpression(unitExpression), m_exponent(exponent)
    {
    }

    DynamicUnitExpression::ValuePointer DynamicUnitExpressionAddExponentToLastTerm::evaluate(ScriptContext const &ctx) const
    {
        try
        {
            Runtime::Unit unit = ToUnit(m_unitExpression->evaluate(ctx));
            int exponent = m_exponent->evaluate(ctx)->toInt();

            return FromUnit(AddExponentToLastTerm(unit, exponent));
        }
        catch (ixion::no_location_javascript_exception &half)
        {
            throw ixion::javascript_exception(half, getCodeLocation());
        }
    }

    std::string DynamicUnitExpressionAddExponentToLastTerm::toString(int indent) const
    {
        return m_unitExpression->toString(indent) + "^" + m_exponent->toString(0);
    }

}}}}