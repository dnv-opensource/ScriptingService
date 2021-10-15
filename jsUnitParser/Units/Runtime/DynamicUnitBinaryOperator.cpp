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
#include "DynamicUnitBinaryOperator.h"

namespace DNVS {namespace MoFa { namespace Units { namespace Parser {

    DynamicUnitBinaryOperator::DynamicUnitBinaryOperator(IUnitHelper& helper, OperatorId id, ExpressionPointer lhs, ExpressionPointer rhs,
                                                         CodeLocation const &loc)
        : DynamicUnitExpression(helper, loc), m_id(id), m_lhs(lhs), m_rhs(rhs)
    {
    }

    DynamicUnitExpression::ValuePointer DynamicUnitBinaryOperator::evaluate(ScriptContext const &ctx) const
    {
        try
        {
            Runtime::Unit result_lhs = ToUnit(m_lhs->evaluate(ctx));
            Runtime::Unit result_rhs = ToUnit(m_rhs->evaluate(ctx));

            if (m_id == ixion::javascript::value::OP_MULTIPLY)
                return FromUnit(result_lhs * result_rhs);
            else if (m_id == ixion::javascript::value::OP_DIVIDE)
                return FromUnit(result_lhs / result_rhs);

            EXJS_THROWINFO_NO_LOCATION(ECJS_INVALID_OPERATION, "Only multiplications and divisions allowed")
        }
        catch (ixion::no_location_javascript_exception &half)
        {
            throw ixion::javascript_exception(half, getCodeLocation());
        }
    }

    std::string DynamicUnitBinaryOperator::toString(int indent) const
    {
        return m_lhs->toString(indent) + ixion::javascript::value::operator2string(m_id) + m_rhs->toString(0);
    }

}}}}