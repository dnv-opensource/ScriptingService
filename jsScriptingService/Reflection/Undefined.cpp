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
#include "Undefined.h"
#include "Reflection\Classes\Class.h"
#include "Reflection\Members\GlobalType.h"

namespace DNVS { namespace MoFa { namespace Scripting {

    bool operator==(const Object& lhs, const Undefined& rhs)
    {
        return false;
    }
    bool operator!=(const Object& lhs, const Undefined& rhs)
    {
        return true;
    }

    bool Undefined::operator==(const Undefined& other) const
    {
        return true;
    }

    bool Undefined::operator!=(const Undefined& other) const
    {
        return false;
    }

    bool Undefined::operator==(const Object& other) const
    {
        return false;
    }

    bool Undefined::operator==(std::nullptr_t) const
    {
        return true;
    }


    bool Undefined::operator!=(const Object& other) const
    {
        return true;
    }

    bool Undefined::operator!=(std::nullptr_t) const
    {
        return false;
    }

    bool operator==(std::nullptr_t, const Undefined& rhs)
    {
        return true;
    }
    bool operator!=(std::nullptr_t, const Undefined& rhs)
    {
        return false;
    }

    void DoReflect(const TypeLibraryPointer& typeLibrary, Undefined**)
    {
        using namespace DNVS::MoFa::Reflection;
        using namespace DNVS::MoFa::Reflection::Classes;
        Class<Undefined, Undefined> cls(typeLibrary, "undefined");
        cls.Operator(This.Const == This.Const);
        cls.Operator(This.Const != This.Const);
        cls.Operator(This.Const == Object());
        cls.Operator(This.Const != Object());
        cls.Operator(Object() == This.Const);
        cls.Operator(Object() != This.Const);
        cls.Operator(This.Const == nullptr);
        cls.Operator(nullptr == This.Const);
        cls.Operator(This.Const != nullptr);
        cls.Operator(nullptr != This.Const);
        cls.ImplicitConversion([](const Undefined&) {return std::string("undefined"); });
        cls.Function("toString", [](const Undefined&) {return std::string("undefined"); });

        Class<Members::GlobalType> global(typeLibrary, "");
        global.StaticFunction("isNaN", [](const Undefined&) {return true; });
        global.StaticFunction("isFinite", [](const Undefined&) {return false; });
    }

}}}

