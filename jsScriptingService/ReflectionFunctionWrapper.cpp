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
#include "ReflectionFunctionWrapper.h"
#include "Ixion\reflected_delegate.h"
#include "Ixion\ref_ptr.Reflection.h"

namespace DNVS { namespace MoFa { namespace Scripting {

    ReflectionFunctionWrapper::ReflectionFunctionWrapper(const Reflection::TypeLibraries::TypeLibraryPointer& typeLibrary)
        : m_typeLibrary(typeLibrary)
    {}

    Reflection::Objects::Object ReflectionFunctionWrapper::WrapFunction(const Reflection::Objects::Object& object, const std::string& method)
    {
        return Reflection::Objects::Object(m_typeLibrary, ixion::ref_ptr<ixion::javascript::value>(new ixion::javascript::reflected_delegate(object, method)));
    }

    bool ReflectionFunctionWrapper::TryUnwrapFunction(const Reflection::Objects::Object& function, Reflection::Objects::Object& object, std::string& method)
    {
        if (function.IsConvertibleTo<ixion::ref_ptr<ixion::javascript::reflected_delegate, ixion::javascript::value>>())
        {
            auto delegate = function.As<ixion::ref_ptr<ixion::javascript::reflected_delegate, ixion::javascript::value>>();
            object = delegate->GetObject();
            method = delegate->GetName();
            return true;
        }
        return false;
    }

}}}


