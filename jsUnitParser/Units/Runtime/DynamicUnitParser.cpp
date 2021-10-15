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
#include "DynamicUnitParser.h"
#include "DynamicUnitLookup.h"
#include "DynamicUnitBinaryOperator.h"
#include "DynamicQuantityExpression.h"
#include "DynamicUnitExpressionAddExponentToLastTerm.h"

namespace DNVS {namespace MoFa { namespace Units { namespace Parser {

    DynamicUnitParser::DynamicUnitParser(const std::shared_ptr<IUnitHelper>& helper)
        : m_helper(helper)
    {
    }

    DynamicUnitParser::ExpressionPointer DynamicUnitParser::Parse(ExpressionPointer expr, TokenIterator& first,
        TokenIterator const& last, int precedence)
    {
        CodeLocation loc(*first);
        ExpressionPointer unitExpression = ParseUnit(first, last, precedence);

        if (unitExpression)
            return new DynamicQuantityExpression(*m_helper, expr, unitExpression, loc);
        else
            return NULL;
    }

    IUnitParser::ExpressionPointer DynamicUnitParser::CreateBinaryExpression(OperatorId op, ExpressionPointer lhs,
        ExpressionPointer rhs, CodeLocation const& loc)
    {
        return new DynamicUnitBinaryOperator(*m_helper, op, lhs, rhs, loc);
    }

    IUnitParser::ExpressionPointer DynamicUnitParser::CreateExponentExpression(ExpressionPointer expr,
        ExpressionPointer exponent,
        CodeLocation const& loc)
    {
        return new DynamicUnitExpressionAddExponentToLastTerm(*m_helper, expr, exponent, loc);
    }

    IUnitParser::ExpressionPointer DynamicUnitParser::CreateUnitExpression(const std::string& text, CodeLocation const& loc)
    {
        using namespace DNVS::MoFa::Units::Runtime;
        Unit unit = m_dynamicUnitParser.Parse(text);

        if (!unit.IsValid())
            return ExpressionPointer();
        else
            return new DynamicUnitLookup(*m_helper, unit, loc);
    }

}}}}