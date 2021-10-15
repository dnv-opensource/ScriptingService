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
#include "jsScriptingLValueService.h"
#include "Reflection\Types\DynamicTypeTraits.h"
#include "Reflection\LValue.h"


using namespace DNVS::MoFa::Reflection;

jsScriptingLValueService::jsScriptingLValueService()
{
}

void jsScriptingLValueService::AddReference(LValue& lvalue)
{
    auto dynamicObject = GetObject(lvalue);
    if (!Types::IsPointer(dynamicObject.GetDecoratedTypeInfo()))
        return;

    m_lvalues.insert(&lvalue);
    m_objectToLValues[dynamicObject].insert(&lvalue);
}

void jsScriptingLValueService::RemoveReference(LValue& lvalue)
{
    auto it = m_objectToLValues.find(GetObject(lvalue));
    if (it != m_objectToLValues.end())
    {
        it->second.erase(&lvalue);
        if (it->second.empty())
        {
            m_objectToLValues.erase(it);
        }
    }
    m_lvalues.erase(&lvalue);
}

void jsScriptingLValueService::DeleteObject(const Object& object)
{
    auto it = m_objectToLValues.find(object.ConvertToDynamicType());
    if (it != m_objectToLValues.end())
    {
        auto lvalues = it->second;
        for (LValue* lvalue : lvalues)
        {
            lvalue->Reset();
        }
    }
}

size_t jsScriptingLValueService::CountLValues(const Object& object)
{
    auto it = m_objectToLValues.find(object.ConvertToDynamicType());
    if (it != m_objectToLValues.end())
        return it->second.size();
    else
        return 0;
}

void jsScriptingLValueService::Clear()
{
    m_lvalues.clear();
    m_objectToLValues.clear();
}

Object jsScriptingLValueService::GetObject(LValue& lvalue) const
{
    return lvalue.GetObject().ConvertToDynamicType();
}
