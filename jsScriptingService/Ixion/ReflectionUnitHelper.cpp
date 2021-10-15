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
#include "ReflectionUnitHelper.h"
#include "Reflection/Objects/Object.h"
#include "reflected_value.h"
#include "ref_ptr.Reflection.h"

namespace DNVS { namespace MoFa { namespace Units { namespace Parser {

    ReflectionUnitHelper::ReflectionUnitHelper(const Reflection::TypeLibraries::TypeLibraryPointer& typeLibrary)
        : m_typeLibrary(typeLibrary)
    {
    }

    Runtime::Unit ReflectionUnitHelper::ToUnit(ValuePointer value) const
    {
        return Reflection::Objects::Object(m_typeLibrary, value).As<Runtime::Unit>();
    }

    Runtime::DynamicQuantity ReflectionUnitHelper::ToQuantity(ValuePointer value) const
    {
        return Reflection::Objects::Object(m_typeLibrary, value).As<Runtime::DynamicQuantity>();
    }

    IxionTypedefs::ValuePointer ReflectionUnitHelper::FromUnit(const Runtime::Unit& unit) const
    {
        return new ixion::javascript::reflected_value(Reflection::Objects::Object(m_typeLibrary, unit));
    }

    IxionTypedefs::ValuePointer ReflectionUnitHelper::FromQuantity(const Runtime::DynamicQuantity& quantity) const
    {
        return new ixion::javascript::reflected_value(Reflection::Objects::Object(m_typeLibrary, quantity));
    }

}}}}

