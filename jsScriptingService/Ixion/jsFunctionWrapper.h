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
#include "ixlib_javascript.hh"
#include "jsScript\jsTypeLibrary.h"
#include "jsScript\jsFunction.h"
#include "ixlib_js_internals.hh"

namespace ixion {namespace javascript {
    class jsFunctionWrapper : public jsFunction
    {
    public:
        jsFunctionWrapper(jsTypeLibrary& typeLibrary, ref_ptr<callable_with_parameters,value> function);
        virtual jsValue* call(const std::vector<jsValue*>& params);
        virtual std::string toString();
        virtual std::string typeName();
        virtual int param_size();
		virtual std::string param_value(int i);
    private:
        ref_ptr<callable_with_parameters, value> m_function;
    };
}}