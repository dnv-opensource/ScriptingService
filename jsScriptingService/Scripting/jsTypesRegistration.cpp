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
#include "jsTypesRegistration.h"
#include "jsScript\jsMath.h"
#include "jsScript\jsModelObject.h"
#include "jsUnits\jsUnits.h"
#include "jsScript\jsScriptable.h"
#include "Reflection\TypeConversions\InheritanceConversions.h"
#include "Reflection\TypeConversions\DynamicTypeDeduction.h"
#include "Reflection\TypeLibraries\ITypeLibrary.h"
#include "jsScript\jsCollections.h"
#include "jsScript\jsConstructor.h"
#include "jsScript\jsArray.h"
#include "jsScript\jsClass.h"

void RegisterJsTypes(jsTypeLibrary& typeLibrary)
{
    jsModelObject::init(typeLibrary);
    jsMath::init(typeLibrary);
    jsUnits::init(typeLibrary);
    jsArray::init(typeLibrary, false);
    using namespace DNVS::MoFa::Reflection::TypeConversions;
    AddDynamicTypeDeduction<jsValue>(typeLibrary.GetReflectionTypeLibrary()->GetConversionGraph());
    AddInheritanceConversions<jsValue, jsScriptable>(typeLibrary.GetReflectionTypeLibrary()->GetConversionGraph());

    AddInheritanceConversions<jsValue, jsConstructorCollection>(typeLibrary.GetReflectionTypeLibrary()->GetConversionGraph());
    AddInheritanceConversions<jsValue, jsFunctionCollection>(typeLibrary.GetReflectionTypeLibrary()->GetConversionGraph());

    AddInheritanceConversions<jsValue, jsFunction>(typeLibrary.GetReflectionTypeLibrary()->GetConversionGraph());
    AddInheritanceConversions<jsValue, jsConstructor>(typeLibrary.GetReflectionTypeLibrary()->GetConversionGraph());
}
