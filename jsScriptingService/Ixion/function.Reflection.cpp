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
#include "function.Reflection.h"
#include "ixlib_js_internals.hh"
#include "Reflection\Classes\Class.h"
#include "value.Reflection.h"
#include "jsFunctionWrapper.h"
#include "ref_ptr.Reflection.h"
#include "Reflection\TypeConversions\InheritanceConversions.h"

namespace ixion {namespace javascript {

    void DoReflect(TypeLibraryPointer typeLibrary, jsTypeLibrary& typeLibraryJs, function**)
    {
        using namespace DNVS::MoFa::Reflection::Classes;
        Class<function, Public<callable_with_parameters>, ref_ptr<function, value>> cls(typeLibrary, "function");
        cls.ImplicitConversion(
            [&typeLibraryJs](ref_ptr<function, value> fn) { return mofa::ref<jsFunction>(new jsFunctionWrapper(typeLibraryJs, fn)); },
            Alias
        );
    }

    void DoReflect(TypeLibraryPointer typeLibrary, jsTypeLibrary& typeLibraryJs, method**)
    {
        using namespace DNVS::MoFa::Reflection::Classes;
        Class<method, Public<callable_with_parameters>, ref_ptr<method, value>> cls(typeLibrary, "method");
        cls.ImplicitConversion(
            [&typeLibraryJs](ref_ptr<method, value> fn) { return mofa::ref<jsFunction>(new jsFunctionWrapper(typeLibraryJs, fn)); },
            Alias
        );
    }

    void DoReflect(TypeLibraryPointer typeLibrary, jsTypeLibrary& typeLibraryJs, constructor**)
    {
        using namespace DNVS::MoFa::Reflection::Classes;
        Class<constructor, Public<callable_with_parameters>, ref_ptr<constructor, value>> cls(typeLibrary, "constructor");
        cls.ImplicitConversion(
            [&typeLibraryJs](ref_ptr<constructor, value> fn) { return mofa::ref<jsFunction>(new jsFunctionWrapper(typeLibraryJs, fn)); },
            Alias
        );
    }

    void DoReflect(TypeLibraryPointer typeLibrary, callable_with_parameters**)
    {
        using namespace DNVS::MoFa::Reflection::Classes;
        Class<callable_with_parameters, Public<value>, ref_ptr<callable_with_parameters, value>> cls(typeLibrary, "callable_with_parameters");
        DNVS::MoFa::Reflection::TypeConversions::AddInheritanceConversions<jsFunction, jsFunctionWrapper>(typeLibrary->GetConversionGraph());
    }

}}

