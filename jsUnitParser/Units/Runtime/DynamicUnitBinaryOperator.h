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
#include "DynamicUnitExpression.h"

namespace DNVS {namespace MoFa { namespace Units { namespace Parser {

    class DynamicUnitBinaryOperator : public DynamicUnitExpression
    {
    public:
        DynamicUnitBinaryOperator(IUnitHelper& helper, OperatorId id, ExpressionPointer lhs, ExpressionPointer rhs, CodeLocation const &loc);
        ValuePointer evaluate(ScriptContext const &ctx) const;
        std::string toString(int indent) const;

    private:
        ExpressionPointer m_lhs;
        ExpressionPointer m_rhs;
        OperatorId m_id;
    };

}}}}