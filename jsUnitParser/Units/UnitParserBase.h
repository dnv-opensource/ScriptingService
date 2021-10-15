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

#include "ixlib_js_internals.hh"
#include "IUnitParser.h"

namespace DNVS {namespace MoFa { namespace Units { namespace Parser {

    class UnitParserBase : public IUnitParser
    {
    public:
        void Advance(ConstTokenIterator& first, ConstTokenIterator const& last);
        ExpressionPointer ParseUnit(ConstTokenIterator& first, ConstTokenIterator const& last, int precedence);
        ExpressionPointer ParseExponent(ConstTokenIterator& first, ConstTokenIterator const& last);

        virtual ExpressionPointer CreateBinaryExpression(OperatorId op, ExpressionPointer lhs, ExpressionPointer rhs,
                                                         CodeLocation const& loc) = 0;
        virtual ExpressionPointer CreateExponentExpression(ExpressionPointer expr, ExpressionPointer exponent,
                                                           CodeLocation const& loc) = 0;
        virtual ExpressionPointer CreateUnitExpression(const std::string& text, CodeLocation const& loc) = 0;
    };

}}}}