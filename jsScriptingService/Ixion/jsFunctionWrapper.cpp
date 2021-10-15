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
#include "jsFunctionWrapper.h"
#include "ConversionHelper.h"
#include "ref_ptr.Reflection.h"

namespace ixion {namespace javascript {

    jsFunctionWrapper::jsFunctionWrapper(jsTypeLibrary& typeLibrary, ref_ptr<callable_with_parameters, value> function)
        : jsFunction(typeLibrary)
        , m_function(function)
    {
    }

    jsValue* jsFunctionWrapper::call(const std::vector<jsValue*>& params)
    {
        value::parameter_list ixparams;

        for (size_t i = 0; i < params.size(); ++i)
            ixparams.push_back(ConversionHelper::ToIxion(params[i]));

        auto result = m_function->call(ixparams);
        if (result)
        {
            mofa::ref<jsValue> value = DNVS::MoFa::Reflection::Objects::Object(GetTypeLibrary().GetReflectionTypeLibrary(), result).As<mofa::ref<jsValue>>();
            jsStack::stack()->insert(value);
            return value;
        }
        return nullptr;
    }

    std::string jsFunctionWrapper::toString()
    {
        return m_function->stringify();
    }

    std::string jsFunctionWrapper::typeName()
    {
        return m_function->toString();
    }

    int jsFunctionWrapper::param_size()
    {
        return (int)m_function->param_count();
    }

	std::string jsFunctionWrapper::param_value(int i)
	{
		return m_function->GetParameterNames()[i];
	}

}}

