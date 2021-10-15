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
#include "Reflection\Objects\Object.h"

namespace DNVS { namespace MoFa { namespace Scripting {
    using Reflection::Objects::Object;
    using DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer;
    class LValue
    {
    public:
        LValue();
        LValue(const LValue& other);
        LValue(LValue&& other);
        LValue(const Object& object);
        LValue& operator=(const LValue& object);
        ~LValue();
        Object operator!=(const Object& other) const;
        LValue& operator=(const Object& other);
        LValue& operator+=(const Object& other);
        LValue& operator-=(const Object& other);
        LValue& operator*=(const Object& other);
        LValue& operator/=(const Object& other);
        LValue& operator%=(const Object& other);
        LValue& operator++();
        LValue& operator--();
        LValue operator++(int);
        LValue operator--(int);
        const Object& GetObject() const;
        void Reset();
        void Reset(const Object& other);
    private:
        void RemoveReference();
        void AddReference();
        Object m_object;
    };
    void DoReflect(const TypeLibraryPointer& typeLibrary, LValue**);
}}}
