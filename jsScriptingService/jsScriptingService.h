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
#include "Scripting\IScriptingService.h"
#include "Scripting/INameService.h"
#include "TypeUtilities/NonNullablePtr.h"
#include <functional>
class jsTypeLibrary;
class jsScriptingServiceImpl;

#pragma warning(push)
#pragma warning(disable:4275)
class jsScriptingService : public DNVS::MoFa::Scripting::IScriptingService
{
public:
    jsScriptingService(const std::shared_ptr<jsTypeLibrary>& typeLibrary, std::shared_ptr<DNVS::MoFa::Scripting::INameService> nameService, const std::function<void(int)>& lineNumberCallback = nullptr);
    ~jsScriptingService();
    DNVS::MoFa::Reflection::Objects::Object Execute(const std::string& str, bool logging = false) override;
    DNVS::MoFa::Reflection::Objects::Object Execute(std::istream& istr, bool logging = false) override;
    DNVS::MoFa::Reflection::Objects::Object Test(const std::string& str) override;
    DNVS::MoFa::Reflection::Objects::Object Invoke(const std::string& expression, const std::vector<DNVS::MoFa::Reflection::Objects::Object>& arguments) override;

    DNVS::MoFa::Reflection::TypeConversions::ConversionGraphPointer GetConversionGraph() const override;
    DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer GetTypeLibrary() const override;
    DNVS::MoFa::Services::Allocators::ForwardingAllocatorSelector GetAllocator() const override;
    bool IsLookupExpression(const std::string& expr) const override;
    bool TrySplitFunctionArguments(const std::string& expr, std::string& functionName, std::vector<std::string>& arguments) const override;
    bool HasIdentifier(const std::string& name) const override;
    bool HasMember(const std::string& name) const override;
    void DeleteMember(const std::string& name) override;
    void RenameMember(const DNVS::MoFa::Reflection::Objects::Object& object, const std::string& newName, bool forceRename = false) override;
    bool InAssignment() const override;
    void ClearMembers() override;
    std::set<std::string> GetAllMemberNames() const override;

    std::shared_ptr<DNVS::MoFa::Scripting::IAutoCompletion> TryAutoComplete(const std::string& text, bool functionCompletion = true) const override;
    DNVS::MoFa::Scripting::ExecutionStatus GetExecutionStatus() const;

    DNVS::MoFa::Reflection::Objects::Object Lookup(const std::string& name) override;
    bool TryGetName(const DNVS::MoFa::Reflection::Objects::Object& object, std::string& name) override;

private:
    std::shared_ptr<jsTypeLibrary> m_typeLibrary;
    DNVS::MoFa::TypeUtilities::NonNullablePtr<jsScriptingServiceImpl> m_impl;
    DNVS::MoFa::Scripting::ExecutionStatus m_executionStatus;
};
#pragma warning(pop)