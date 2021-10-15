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
#include "IxionAutoCompletionContext.h"
#include "extended_list_scope.h"

namespace ixion {namespace javascript {
    using namespace DNVS::MoFa;
    IxionAutoCompletionContext::IxionAutoCompletionContext(extended_list_scope& scope)
        : m_scope(scope)
        , m_globalContext(true)
    {

    }

    bool IxionAutoCompletionContext::IsGlobalContext() const
    {
        return m_globalContext;
    }

    void IxionAutoCompletionContext::CollectMembers(std::shared_ptr<jsAutoCompletion> autoComplete) const
    {
        if (IsGlobalContext())
        {
            for (const auto& name : m_scope.GetNames())
                autoComplete->addVariable(name);
        }
    }

    void IxionAutoCompletionContext::CollectFunctions(const Reflection::Objects::Object& function, std::shared_ptr<jsAutoCompletion> autoComplete) const
    {
    }

    bool IxionAutoCompletionContext::HasContext() const
    {
        return m_globalContext;
    }

    Reflection::Objects::Object IxionAutoCompletionContext::lookup(const std::string& function)
    {
        return Reflection::Objects::Object();
    }

    void IxionAutoCompletionContext::SetNestedContext(const Reflection::Objects::Object& nestedContext)
    {
        m_globalContext = false;
    }

    void IxionAutoCompletionContext::SetGlobalContext()
    {
        m_globalContext = true;
    }

}}

