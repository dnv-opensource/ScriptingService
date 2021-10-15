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
#include "UnitParserBase.h"
#include "ixlib_token_javascript.hh"
#include "ixlib_numconv.hh"

namespace DNVS {namespace MoFa { namespace Units { namespace Parser {

void UnitParserBase::Advance(ConstTokenIterator& first, ConstTokenIterator const& last)
{
    ++first;

    if (first == last)
        EXJS_THROW(ECJS_UNEXPECTED_EOF)
}

IxionTypedefs::ExpressionPointer UnitParserBase::ParseUnit(ConstTokenIterator& first, ConstTokenIterator const& last,
                                                           int precedence)
{
    if (first == last)
        return ExpressionPointer();

    CodeLocation loc(*first);
    ExpressionPointer unitExpression = CreateUnitExpression(first->Text, loc);

    if (unitExpression)
    {
        ++first;
        if (first == last)
            return unitExpression;
    }
    else
        return ExpressionPointer();

    bool parsed_something;
    do
    {
        parsed_something = false;

        if (first->Type == '(')
        {
            --first;
            // Units can not be called. This symbol cannot be a unit after all.
            return ExpressionPointer();
        }

        if (first->Type == '^' && precedence <= PreceedencePower)
        {
            CodeLocation loc(*first);
            ExpressionPointer exponent = ParseExponent(first, last);
            unitExpression = CreateExponentExpression(unitExpression, exponent, loc);

            if (first == last)
                return unitExpression;

            parsed_something = true;
        }
        else if ((first->Type == '*' || first->Type == '/') && precedence < PreceedenceMultiplication)
        {
            CodeLocation loc(*first);
            OperatorId op = ixion::javascript::value::token2operator(*first);

            Advance(first, last);

            ExpressionPointer right = ParseUnit(first, last, PreceedenceMultiplication);
            if (right.get() == NULL)
            {
                first--;
                return unitExpression;
            }
            else
                unitExpression = CreateBinaryExpression(op, unitExpression, right, loc);

            parsed_something = true;
        }
    } while (first != last && parsed_something);

    return unitExpression;
}

IxionTypedefs::ExpressionPointer UnitParserBase::ParseExponent(ConstTokenIterator& first,
                                                               ConstTokenIterator const& last)
{
    CodeLocation loc(*first);
    OperatorId op = ixion::javascript::value::token2operator(*first);

    Advance(first, last);

    int sign = 1;
    if (first->Type == '-')
    {
        sign = -1;
        Advance(first, last);
    }

    if (first->Type == TT_JS_LIT_INT)
    {
        ExpressionPointer exponent = new ixion::javascript::constant(
            ixion::javascript::makeConstant(sign * ixion::evalUnsigned(first->Text)), *first);
        ++first;

        return exponent;
    }
    else
        EXJS_THROWINFO(ECJS_UNEXPECTED, (first->Text + " instead of integer").c_str())
}

}}}}