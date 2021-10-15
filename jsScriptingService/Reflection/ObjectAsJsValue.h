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
#include "jsScript\jsValue.h"

namespace DNVS { namespace MoFa { namespace Scripting {
    //This object handles the problem that jsScript depends on all objects be
    class ObjectAsJsValue : public jsValue
    {
    public:
        ObjectAsJsValue(const Reflection::Objects::Object& object);
        Reflection::Objects::Object GetObject() const;
        virtual jsValue* operator!();
        virtual jsValue* operator-();
        virtual jsValue* operator/(jsValue* op2);
        virtual jsValue* operator+(jsValue* op2) override;
        virtual jsValue* operator-(jsValue* op2) override;
        virtual jsValue* operator*(jsValue* op2) override;
        virtual jsValue* operator==(jsValue* op2) override;
        virtual jsValue* operator!=(jsValue* op2) override;
        virtual jsValue* operator<=(jsValue* op2) override;
        virtual jsValue* operator>=(jsValue* op2) override;
        virtual jsValue* operator<(jsValue* op2) override;
        virtual jsValue* operator>(jsValue* op2) override;
        virtual std::string typeName();
    private:
        Reflection::Objects::Object m_object;
    };
    void DoReflect(const Reflection::TypeLibraries::TypeLibraryPointer& typeLibrary, ObjectAsJsValue**);
}}}