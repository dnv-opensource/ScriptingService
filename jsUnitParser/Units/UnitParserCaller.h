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
#include "IxionTypedefs.h"

namespace DNVS {namespace MoFa { namespace Units { namespace Parser {
    class IUnitParser;
    template<typename T>
    struct UnitParserCaller : public IxionTypedefs
    {
        UnitParserCaller(UnitParserCaller&& other) = default;
        UnitParserCaller(const UnitParserCaller& other) = default;
        template<typename... Args>
        UnitParserCaller(Args&&... args) : m_parser(std::make_shared<T>(std::forward<Args>(args)...))
        {
        }
        ExpressionPointer operator()(ExpressionPointer expr, TokenIterator& first,
            TokenIterator const& last, int precedence)
        {
            return m_parser->Parse(expr, first, last, precedence);
        }
    private:
        std::shared_ptr<IUnitParser> m_parser;
    };
}}}}