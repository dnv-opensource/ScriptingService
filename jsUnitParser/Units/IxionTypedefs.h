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

namespace DNVS {namespace MoFa { namespace Units { namespace Parser {

    class IxionTypedefs
    {
    public:
        typedef ixion::ref_ptr<ixion::javascript::expression> ExpressionPointer;
        typedef ixion::scanner::token_iterator TokenIterator;
        typedef ixion::scanner::const_token_iterator ConstTokenIterator;
        typedef ixion::javascript::code_location CodeLocation;
        typedef ixion::javascript::value Value;
        typedef ixion::ref_ptr<Value> ValuePointer;
        typedef ixion::javascript::context ScriptContext;
        typedef ixion::javascript::value::operator_id OperatorId;

        enum OperatorPreceedenceEnum
        {
            PreceedenceMultiplication = 140,  // * / % [ok]
            PreceedencePower = 150,           // ^ when compiling units
        };
    };

}}}}