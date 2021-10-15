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
#include "js_class_instance.Reflection.h"
#include "ixlib_js_internals.hh"
#include "Reflection/Classes/Class.h"
#include "value.Reflection.h"

namespace ixion {namespace javascript {

    void DoReflect(TypeLibraryPointer typeLibrary, jsTypeLibrary& typeLibraryJs, js_class_instance**)
    {
        using namespace DNVS::MoFa::Reflection::Classes;
        Class<js_class_instance, Public<value>, ref_ptr<js_class_instance, value>> cls(typeLibrary, "js_class_instance");
    }

}}

