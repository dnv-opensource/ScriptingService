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
#include "jsScript/AutoCompletion/IAutoCompletionContext.h"
namespace ixion {namespace javascript {
    class extended_list_scope;
    class IxionAutoCompletionContext : public DNVS::MoFa::Scripting::IAutoCompletionContext 
    {
    public:
        IxionAutoCompletionContext(extended_list_scope& scope);

        virtual bool IsGlobalContext() const override;


        virtual void CollectMembers(std::shared_ptr<jsAutoCompletion> autoComplete) const override;


        virtual void CollectFunctions(const DNVS::MoFa::Reflection::Objects::Object& function, std::shared_ptr<jsAutoCompletion> autoComplete) const override;


        virtual bool HasContext() const override;


        virtual DNVS::MoFa::Reflection::Objects::Object lookup(const std::string& function) override;


        virtual void SetNestedContext(const DNVS::MoFa::Reflection::Objects::Object& nestedContext) override;


        virtual void SetGlobalContext() override;

    private:
        extended_list_scope& m_scope;
        bool m_globalContext;
    };
}}