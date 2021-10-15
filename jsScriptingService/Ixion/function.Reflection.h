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
#include "Reflection\TypeLibraries\TypeLibraryPointer.h"

class jsTypeLibrary;

namespace ixion {namespace javascript {
    class function;
    class method;
    class constructor;
    class callable_with_parameters;

    using DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer;
    void DoReflect(TypeLibraryPointer typeLibrary, jsTypeLibrary& typeLibraryJs, function**);
    void DoReflect(TypeLibraryPointer typeLibrary, jsTypeLibrary& typeLibraryJs, method**);
    void DoReflect(TypeLibraryPointer typeLibrary, jsTypeLibrary& typeLibraryJs, constructor**);
    void DoReflect(TypeLibraryPointer typeLibrary, callable_with_parameters**);
}}