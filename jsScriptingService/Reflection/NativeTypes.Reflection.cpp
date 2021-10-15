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
#include "Reflection/Reflect.h"
#include "Reflection/TypeConversions/BuiltInConversions.h"
#include "Reflection/Objects/Object.h"

#include "NativeTypes.Reflection.h"
#include "LValue.h"
#include "Array.h"
#include "Undefined.h"
#include "Null.h"
#include "String.h"
#include "Number.h"
#include "ObjectAsJsValue.h"
#include "ReflectUnits.h"
#include "jsScript/Reflection/jsValue.Reflection.h"
#include "jsScript/Reflection/jsArray.Reflection.h"
#include "Reflection/Utilities/DefaultRegistration.h"

void ReflectNative(TypeLibraries::TypeLibraryPointer typeLibrary)
{
    TypeConversions::AddBuiltInConversions(typeLibrary->GetConversionGraph());
    using namespace DNVS::MoFa::Reflection;
    Reflect<DNVS::MoFa::Reflection::Objects::Object>(typeLibrary);
    DNVS::MoFa::Reflection::Utilities::DefaultRegistration::Reflect(typeLibrary);
    Reflect<std::string>(typeLibrary);
    Reflect<int>(typeLibrary);
    Reflect<long>(typeLibrary);
    Reflect<unsigned int>(typeLibrary);
    Reflect<long int>(typeLibrary);
    Reflect<unsigned long>(typeLibrary);
    Reflect<unsigned __int64>(typeLibrary);
    Reflect<__int64>(typeLibrary);
    Reflect<double>(typeLibrary);
    Reflect<char>(typeLibrary);
    Reflect<unsigned char>(typeLibrary);
    Reflect<signed char>(typeLibrary);
    Reflect<float>(typeLibrary);
    Reflect<short>(typeLibrary);
    Reflect<unsigned short>(typeLibrary);
    Reflect<bool>(typeLibrary);
    Reflect<DNVS::MoFa::Scripting::LValue>(typeLibrary);
    Reflect<DNVS::MoFa::Scripting::Array>(typeLibrary);
    Reflect<DNVS::MoFa::Scripting::Undefined>(typeLibrary);
    Reflect<std::nullptr_t>(typeLibrary);
    Reflect<DNVS::MoFa::Scripting::ObjectAsJsValue>(typeLibrary);
    Reflect<jsQuantity>(typeLibrary);
    Reflect<jsUnitValue>(typeLibrary);
    Reflect<jsReference>(typeLibrary);
    DNVS::MoFa::Units::ReflectUnits(typeLibrary);
}