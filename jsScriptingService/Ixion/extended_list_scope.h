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
#include "Reflection/TypeLibraries/TypeLibraryPointer.h"
#include "IValueCreatorService.h"
#include "jsScript/jsModelObject.h"

using namespace DNVS::MoFa::Reflection;
class jsTypeLibrary;

namespace DNVS { namespace MoFa { namespace Scripting {
    class INewObjectScope;
}}}
namespace ixion { namespace javascript {
    class extended_list_scope : public list_scope
    {
    public:
        extended_list_scope(jsTypeLibrary& typeLibrary, const std::shared_ptr<IValueCreatorService>& valueCreation);
        ~extended_list_scope();
        ref_ptr<list_scope, value> construct();
        ref_ptr<value> operatorBinaryModifying(context const& ctx, ref_ptr<expression> opn1, operator_id op,
            ref_ptr<expression> opn2);

        virtual bool hasMember(std::string const &name) const override;
        virtual void addMember(std::string const &name, ref_ptr<value> member) override;
        virtual void removeMember(std::string const &name) override;
        virtual void clearMembers() override;
        virtual ref_ptr<value> defineMember(context const& ctx, const std::string& name, ref_ptr<expression> expression, const std::shared_ptr<IValueCreatorService>& valueCreatorService);
        virtual ref_ptr<value> assignment(context const &ctx, ref_ptr<expression> opn1, ref_ptr<expression> opn2) override;
        virtual ref_ptr<value> lookup(std::string const &identifier) override;
        const TypeLibraries::TypeLibraryPointer& GetReflectionTypeLibrary() const;
        jsTypeLibrary& GetJsTypeLibrary();
        bool IsInAssignment() const { return m_isInAssignment; }
        bool IsReservedWord(const std::string& identifier) const;
        virtual void setLineNumber(int lineNumber);
        void SetLineNumberHandler(const std::function<void(int lineNumber)>& lineNumberSetter);
        void SetLastLineNumberHandler(const std::function<void(int lineNumber)>& lastLineNumberSetter);
        std::set<std::string> GetNames() const;
        void setLastLineNumber(int lineNumber) override;
    private:
        ref_ptr<value> CreateOrReuseExistingIdentifier(context const& ctx, const std::string& name, const ref_ptr<value>& value);
        ref_ptr<value> CreateIdentifier(const ref_ptr<value>& value);
        ref_ptr<value> AssignAndRename(context const& ctx, const std::string& name, const ref_ptr<expression>& expression, const std::function<ref_ptr<value>(const ref_ptr<value>&)>& createIdentifier);
        std::shared_ptr<IValueCreatorService> m_valueCreation;
        bool m_isInAssignment;
        jsTypeLibrary& m_typeLibrary;      
        mofa::ref<jsModelObject> m_global;
        std::function<void(int lineNumber)> m_lineNumberSetter;
        std::function<void(int lastLineNumber)> m_lastLineNumberSetter;
        void SetNoNameIfNameSameAsIdentifier(ref_ptr<value> member, const std::string& identifier);
        void ConditionallySetName(const std::shared_ptr<DNVS::MoFa::Scripting::INewObjectScope>& newObjectScope, ref_ptr<value> member, const std::string& name);
        void ThrowIfNotValidIdentifier(const std::string& identifier);
    };
}}