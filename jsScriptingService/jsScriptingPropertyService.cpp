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
#include "jsScriptingPropertyService.h"
#include "jsScript\jsProperty.h"
#include "jsScript\jsValueWithOwner.h"
#include "Reflection\ObjectAsJsValue.h"
#include "Ixion\reflected_delegate.h"
#include "Ixion\ref_ptr.Reflection.h"
using namespace DNVS::MoFa::Scripting;

jsProperty* AsProperty(jsValue* value)
{
    jsProperty* prop = dynamic_cast<jsProperty*>(value);
    if (!prop)
    {
        if (jsValueWithOwner* valueWithOwner = dynamic_cast<jsValueWithOwner*>(value))
            prop = dynamic_cast<jsProperty*>(valueWithOwner->GetValue());
    }
    return prop;
}

std::unique_ptr<IScriptingPropertyService::PropertyData> jsScriptingPropertyService::TryGetPropertyData(jsValue* value) const
{
    //1. Make an empty IWrapperService::PropertyData instance. Use unique_ptr to hold it so that we don't need to think of memory cleanup.
    std::unique_ptr<PropertyData> propertyData;
    if (!value)
        //unique_ptrs cannot be copied, so we need to move it in order for this to compile. (Look up move constructor)
        return std::move(propertyData);
    //If value is registered using the old scripting engine, it will be of type jsProperty.
    else if (jsProperty* prop = AsProperty(value))
    {
        //Get metadata about the property, such as if it has a getter and setter and what the return type is.
        propertyData.reset(new PropertyData);
        propertyData->HasGetter = (prop->GetGetter() != nullptr);
        if (propertyData->HasGetter)
        {
            propertyData->IsConst = prop->GetGetter()->IsConst();
            propertyData->Type = prop->GetGetter()->propertyType();
        }
        propertyData->HasSetter = (prop->GetSetter() != nullptr);
    }
    //If value is registered using reflection, it will point to a ReflectedObjectDelegate.
    else if (ObjectAsJsValue* objectAsJsValue = dynamic_cast<ObjectAsJsValue*>(value))
    {
        auto object = objectAsJsValue->GetObject();
    }
    return std::move(propertyData);
}

mofa::ref<jsValue> jsScriptingPropertyService::CreateDelegate(const DNVS::MoFa::Reflection::Objects::Object& object, const std::string& identifier)
{
    ixion::ref_ptr<ixion::javascript::value> delegate(new ixion::javascript::reflected_delegate(object, identifier));
    mofa::ref<jsValue> method = new ObjectAsJsValue(DNVS::MoFa::Reflection::Objects::Object(object.GetTypeLibrary(), delegate));
    jsStack::stack()->insert(method);
    return method;
}