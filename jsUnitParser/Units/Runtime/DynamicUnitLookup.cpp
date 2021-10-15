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
#include "DynamicUnitLookup.h"
#include "Reflection\Objects\Object.h"
#include "jsScript\jsStack.h"

namespace DNVS {namespace MoFa { namespace Units { namespace Parser {

    DynamicUnitLookup::DynamicUnitLookup(IUnitHelper& helper, const Runtime::Unit &unit, CodeLocation const &loc)
        : DynamicUnitExpression(helper, loc), m_unit(unit)
    {
    }
    std::string DynamicUnitLookup::toString(int indent) const
    {
        return getIndent(indent) + m_unit.GetUnitName();
    }

    DynamicUnitExpression::ValuePointer DynamicUnitLookup::evaluate(ScriptContext const &ctx) const
    {
        try
        {
            return FromUnit(m_unit);
        }
        catch (ixion::no_location_javascript_exception &half)
        {
            throw ixion::javascript_exception(half, getCodeLocation());
        }
    }

}}}}