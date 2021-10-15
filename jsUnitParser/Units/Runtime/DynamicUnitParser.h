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
#include "Units/Runtime/UnitParser.h"
#include "../UnitParserBase.h"

namespace DNVS {namespace MoFa { namespace Units { namespace Parser {
    class IUnitHelper;

    class DynamicUnitParser : public UnitParserBase
    {
    public:
        DynamicUnitParser(const std::shared_ptr<IUnitHelper>& helper);
        virtual ExpressionPointer Parse(ExpressionPointer expr, TokenIterator& first, TokenIterator const& last,
                                        int precedence);
        virtual ExpressionPointer CreateBinaryExpression(OperatorId op, ExpressionPointer lhs, ExpressionPointer rhs,
                                                         CodeLocation const& loc);
        virtual ExpressionPointer CreateExponentExpression(ExpressionPointer expr, ExpressionPointer exponent,
                                                           CodeLocation const& loc);
        virtual ExpressionPointer CreateUnitExpression(const std::string& text, CodeLocation const& loc);

    private:
        DNVS::MoFa::Units::Runtime::UnitParser m_dynamicUnitParser;
        std::shared_ptr<IUnitHelper> m_helper;
    };

}}}}