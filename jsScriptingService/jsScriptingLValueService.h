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
#include "Reflection\ILValueService.h"
#include "Reflection\Objects\LessObjectPointer.h"
#include <map>
#include <set>
#include <string>

using DNVS::MoFa::Scripting::LValue;
using DNVS::MoFa::Reflection::Objects::Object;
using DNVS::MoFa::Reflection::Objects::LessObjectPointer;

/*
This class maintains all the LValues that are used in scripting. It also manages the content of these LValues.
LValues act as pointers. They can point to objects.
Two LValues can point to the same objet.
Once all references LValues pointing to an object has been deleted, that object will go out of scope.
*/
class jsScriptingLValueService : public DNVS::MoFa::Scripting::ILValueService
{
public:
    jsScriptingLValueService();
    virtual void AddReference(LValue& lvalue) override;
    virtual void RemoveReference(LValue& lvalue) override;
    virtual void DeleteObject(const Object& object) override;
    virtual size_t CountLValues(const Object& object) override;
    virtual void Clear() override;
private:
    Object GetObject(LValue& lvalue) const;
    std::map<Object, std::set<LValue*>, LessObjectPointer> m_objectToLValues;
    std::set<LValue*> m_lvalues;
};