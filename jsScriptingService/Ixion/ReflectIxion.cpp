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
#include "ReflectIxion.h"
#include "jsValue_value.h"
#include "jsValue_delegate.h"
#include "reflected_delegate.h"
#include "reflected_value.h"
#include "Reflection\Reflect.h"
#include "function.Reflection.h"
#include "js_class_instance.Reflection.h"
#include "dual_delegate.h"

namespace ixion { namespace javascript {

    void ReflectIxion(jsTypeLibrary& typeLibrary)
    {
        using namespace DNVS::MoFa::Reflection;
        Reflect<reflected_value>(typeLibrary.GetReflectionTypeLibrary());
        Reflect<reflected_delegate>(typeLibrary.GetReflectionTypeLibrary());
        Reflect<jsValue_delegate>(typeLibrary.GetReflectionTypeLibrary());
        Reflect<dual_delegate>(typeLibrary.GetReflectionTypeLibrary());
        Reflect<jsValue_value>(typeLibrary.GetReflectionTypeLibrary());
        Reflect<function>(typeLibrary.GetReflectionTypeLibrary(), typeLibrary);
        Reflect<method>(typeLibrary.GetReflectionTypeLibrary(), typeLibrary);
        Reflect<constructor>(typeLibrary.GetReflectionTypeLibrary(), typeLibrary);
        Reflect<js_class_instance>(typeLibrary.GetReflectionTypeLibrary(), typeLibrary);
    }

}}