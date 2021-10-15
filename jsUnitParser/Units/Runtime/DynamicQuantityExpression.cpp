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
#include "DynamicQuantityExpression.h"
namespace DNVS {namespace MoFa { namespace Units { namespace Parser {

    DynamicQuantityExpression::DynamicQuantityExpression(IUnitHelper& helper, ExpressionPointer lhs, ExpressionPointer rhs,
                                                         CodeLocation const &loc)
        : DynamicUnitExpression(helper, loc), m_lhs(lhs), m_rhs(rhs)
    {
    }

    DynamicUnitExpression::ValuePointer DynamicQuantityExpression::evaluate(ScriptContext const &ctx) const
    {
        double doubleValue = m_lhs->evaluate(ctx)->GetDouble();
        Runtime::Unit unit = ToUnit(m_rhs->evaluate(ctx));
        unit.SimplifyUnit();
        return FromQuantity(Runtime::DynamicQuantity(doubleValue, unit));
    }

    std::string DynamicQuantityExpression::toString(int indent) const
    {
        return m_lhs->toString(indent) + " " + m_rhs->toString(0);
    }

}}}}