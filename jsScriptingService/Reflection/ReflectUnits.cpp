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
#include "ReflectUnits.h"
#include "Units\Reflection\ReflectQuantities.h"
#include "Units\Runtime\DynamicQuantity.h"
#include "Reflection\Classes\Class.h"

namespace DNVS {namespace MoFa { namespace Units {

    void ReflectUnits(const DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer& typeLibrary)
    {
        Reflection::ReflectQuantities(typeLibrary);
        Reflection::ReflectDynamicQuantities(typeLibrary);

        using namespace DNVS::MoFa::Reflection::Classes;
        Class<Runtime::DynamicQuantity> cls(typeLibrary, "");
        cls.Function("toDouble", &Runtime::DynamicQuantity::GetValue);
        RegisterToStringFunction(cls);
        cls.ImplicitConversion(&Runtime::DynamicQuantity::GetString);
    }

}}}

