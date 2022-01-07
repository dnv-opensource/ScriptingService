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
#include "ixlib_js_internals.hh"

#include "extended_list_scope.h"
#include "reflected_delegate.h"

#include "Reflection/TypeLibraries/ITypeLibrary.h"
#include "Reflection/Members/IMember.h"
#include "Reflection/Members/GlobalType.h"

#include "TypeUtilities/ScopedVariable.h"
#include "Services/INameManglingService.h"
#include "Services/ServiceProvider.h"
#include "jsScript/jsTypeLibrary.h"
#include "jsValue_delegate.h"
#include "ref_ptr.Reflection.h"
#include "Scripting/INameService.h"
#include "../Reflection/ILValueService.h"
#include "Reflection/Objects/Object.h"
#include "jsScript/jsStack.h"
#include "jsScript/jsTypeInfo.h"
#include "jsScript/jsConstructor.h"
#include "dual_delegate.h"
#include "Services/ScopedServiceRegistration.h"

using namespace DNVS::MoFa::TypeUtilities;
using namespace DNVS::MoFa::Services;
using namespace DNVS::MoFa::Reflection;
using namespace DNVS::MoFa::Scripting;


namespace ixion { namespace javascript {
    extended_list_scope::extended_list_scope(jsTypeLibrary& typeLibrary, const std::shared_ptr<IValueCreatorService>& valueCreation)
        : m_isInAssignment(false)
        , m_typeLibrary(typeLibrary)
        , m_valueCreation(valueCreation)
        , m_global(new jsModelObject)
    {
    }

    extended_list_scope::~extended_list_scope()
    {
        auto nameService = m_typeLibrary.GetReflectionTypeLibrary()->GetServiceProvider().TryGetService<INameService>();
        if (nameService)
        {
            for (const auto& memberPair : MemberMap)
            {
                Objects::Object object(m_typeLibrary.GetReflectionTypeLibrary(), memberPair.second);
                nameService->RemoveObject(object);
            }
        }
    }

    ixion::ref_ptr<ixion::javascript::list_scope, value> extended_list_scope::construct()
    {
        return new extended_list_scope(m_typeLibrary, m_valueCreation);
    }

    ixion::ref_ptr<value> extended_list_scope::operatorBinaryModifying(context const& ctx, ref_ptr<expression> opn1, operator_id op, ref_ptr<expression> opn2)
    {
        lookup_operation* lookup = dynamic_cast<lookup_operation*>(opn1.get());

        if (lookup && lookup->operand() == NULL)
        {
            return list_scope::operatorBinaryModifying(ctx, opn1, op, opn2);
            //             // Throws std::exception if member not found.
//             ref_ptr<value> member = opn1->evaluate(ctx)->operatorBinaryModifying(op, opn2->evaluate(ctx));
//             mofa::ref<jsValue> val = getObject(member);
//             if (member)
//                 member->setName(lookup->identifier());
//             if (val)
//                 val->setName(lookup->identifier());
// 
//             return member;
        }
        else
        {
            return list_scope::operatorBinaryModifying(ctx, opn1, op, opn2);
        }
    }

    bool extended_list_scope::hasMember(std::string const& name) const
    {
        return list_scope::hasMember(name);
    }

    void extended_list_scope::addMember(std::string const &name, ref_ptr<value> member)
    {
        MemberMap[name] = member;
    }

    void extended_list_scope::removeMember(std::string const &name)
    {
        list_scope::removeMember(name);
    }

    void extended_list_scope::clearMembers()
    {
        list_scope::clearMembers();
    }

    ixion::ref_ptr<value> extended_list_scope::defineMember(context const& ctx, const std::string& name, ref_ptr<expression> expression, const std::shared_ptr<IValueCreatorService>& valueCreatorService)
    {
        ref_ptr<value> member = AssignAndRename(ctx, name, expression, [&](const ref_ptr<value> & val) { return CreateIdentifier(val); });
        return member;
    }

    ixion::ref_ptr<value> extended_list_scope::CreateOrReuseExistingIdentifier(context const& ctx, const std::string& name, const ref_ptr<value>& val)
    {
        try
        {
            // Throws std::exception if member not found.
            ref_ptr<value> member = ctx.LookupScope->lookup(name);
            removeMember(name);
            SetNoNameIfNameSameAsIdentifier(member, name);
            member->assign(val);
            return member;
        }
        catch (no_location_javascript_exception&)
        {
            return CreateIdentifier(val);
        }
    }

    ixion::ref_ptr<value> extended_list_scope::CreateIdentifier(const ref_ptr<value>& val)
    {
        return m_valueCreation->MakeLValue(val);
    }
    class JoinedNewObjectScope : public INewObjectScope
    {
    public:
        JoinedNewObjectScope(std::shared_ptr<INewObjectScope> oldScope, std::shared_ptr<INewObjectScope> newScope)
            : m_oldScope(oldScope)
            , m_newScope(newScope)
        {}
        bool IsNewObject(const Objects::Object& object) const override
        {
            if (m_oldScope && m_oldScope->IsNewObject(object))
                return true;
            if (m_newScope && m_newScope->IsNewObject(object))
                return true;
            return false;
        }

        void AddNewObject(const Objects::Object& object) override
        {            
            if (m_oldScope)
                m_oldScope->AddNewObject(object);
            if (m_newScope)
                m_newScope->AddNewObject(object);
        }


        void RenameObject(const Object& object, bool added) override
        {
            if (m_oldScope)
                m_oldScope->RenameObject(object, added);
            if (m_newScope)
                m_newScope->RenameObject(object, added);
        }
    private:
        std::shared_ptr<INewObjectScope> m_oldScope;
        std::shared_ptr<INewObjectScope> m_newScope;
    };
    ixion::ref_ptr<value> extended_list_scope::AssignAndRename(context const& ctx, const std::string& name, const ref_ptr<expression>& expression, const std::function<ref_ptr<value>(const ref_ptr<value>&)>& createIdentifier)
    {
        try
        {
            auto nameService = m_typeLibrary.GetReflectionTypeLibrary()->GetServiceProvider().GetService<INameService>();
            if(!nameService)
                throw std::runtime_error("NameService not defined");
            auto inAssignment = Scope(m_isInAssignment, true);
            std::shared_ptr<INewObjectScope> oldObjectScope = m_typeLibrary.GetReflectionTypeLibrary()->GetServiceProvider().TryGetService<INewObjectScope>();
            std::shared_ptr<INewObjectScope> newObjectScope = nameService->CreateNewObjectScope();
            ScopedServiceRegistration<INewObjectScope> scope(std::make_shared<JoinedNewObjectScope>(oldObjectScope, newObjectScope), m_typeLibrary.GetReflectionTypeLibrary()->GetServiceProvider());
            ref_ptr<value> def;
            if (expression)
                def = expression->evaluate(ctx)->eliminateWrappers()->duplicate();
            else
                def = m_valueCreation->MakeUndefined();
            ref_ptr<value> member = createIdentifier(def);
            // addMember also tries to add the name
            removeMember(name);
            addMember(name, member);
            ConditionallySetName(newObjectScope, member, name);
            return member;
        }
        catch (javascript_exception&)
        {
            throw;
        }
        catch (std::exception& e)
        {
            throw no_location_javascript_exception(ECJS_INVALID_OPERATION, e.what());
        }
        catch (...)
        {
            throw no_location_javascript_exception(ECJS_INVALID_OPERATION, std::exception("Unknown error").what());
        }

    }

    ref_ptr<value> extended_list_scope::assignment(context const& ctx, ref_ptr<expression> opn1, ref_ptr<expression> opn2)
    {
        lookup_operation *lookup = dynamic_cast<lookup_operation*>(opn1.get());
        if (lookup && lookup->operand() == nullptr)
        {
            std::string name = lookup->identifier();
            ThrowIfNotValidIdentifier(name);
            auto defineIdentifier = [&](const ref_ptr<value> & val) {
                return CreateOrReuseExistingIdentifier(ctx, name, val);
            };
            return AssignAndRename(ctx, name, opn2, defineIdentifier);
        }
        else
        {
            auto nameService = m_typeLibrary.GetReflectionTypeLibrary()->GetServiceProvider().GetService<INameService>();
            if (!nameService)
                throw std::runtime_error("NameService not defined");
            std::unique_ptr<INewObjectScope> newObjectScope = nameService->CreateNewObjectScope();
            return list_scope::assignment(ctx, opn1, opn2);
        }
    }
    bool TryUseReflectionMethodInDummyMode(jsValue* method)
    {
        if (!jsStack::stack()->dummyMode())
            return false;
        if (dynamic_cast<jsConstructor*>(method))
            return true;
        return false;
    }
    ref_ptr<value> extended_list_scope::lookup(std::string const &identifier)
    {
        jsValue* method = m_global->lookup(identifier);
        ref_ptr<value> valueDelegate, reflectionDelegate;
        if (method)
            valueDelegate = new jsValue_delegate(m_global, method);

        if (method && !TryUseReflectionMethodInDummyMode(method))
            return valueDelegate;
        TypeLibraries::TypePointer type = GetReflectionTypeLibrary()->LookupType(typeid(Members::GlobalType));
        if (type && type->Lookup(identifier))
        {
            reflectionDelegate = new reflected_delegate(Object(GetReflectionTypeLibrary(), Members::GlobalType()), identifier);
        }
        if (valueDelegate && reflectionDelegate)
            return new dual_delegate(reflectionDelegate, valueDelegate);
        else if (reflectionDelegate)
            return reflectionDelegate;
        else if (valueDelegate)
            return valueDelegate;

        std::string mangledName = identifier;
        if (auto service = ServiceProvider::Instance().TryGetService<INameManglingService>())
            mangledName = service->MangleName(identifier);

        return list_scope::lookup(mangledName);

    }

    const TypeLibraries::TypeLibraryPointer& extended_list_scope::GetReflectionTypeLibrary() const
    {
        return m_typeLibrary.GetReflectionTypeLibrary();
    }

    jsTypeLibrary& extended_list_scope::GetJsTypeLibrary()
    {
        return m_typeLibrary;
    }

    bool extended_list_scope::IsReservedWord(const std::string& identifier) const
    {        
        jsValue* method = m_global->lookup(identifier);
        if (method)
            return true;

        TypeLibraries::TypePointer type = GetReflectionTypeLibrary()->LookupType(typeid(Members::GlobalType));
        if (type && type->Lookup(identifier))
            return true;
        return m_typeLibrary.isReservedWord(identifier);
    }

    void extended_list_scope::setLineNumber(int lineNumber)
    {
        if (m_lineNumberSetter)
            m_lineNumberSetter(lineNumber);
    }

    void extended_list_scope::SetLineNumberHandler(const std::function<void(int lineNumber)>& lineNumberSetter)
    {
        m_lineNumberSetter = lineNumberSetter;
    }

    void extended_list_scope::SetLastLineNumberHandler(const std::function<void(int lineNumber)>& lastLineNumberSetter)
    {
        m_lastLineNumberSetter = lastLineNumberSetter;
    }

    std::set<std::string> extended_list_scope::GetNames() const
    {
        std::set<std::string> names;
        for (const auto& memberPair : MemberMap)
        {
            names.insert(memberPair.first);
        }
        return names;
    }

    void extended_list_scope::setLastLineNumber(int lineNumber)
    {
        if (m_lastLineNumberSetter)
            m_lastLineNumberSetter(lineNumber);
    }

    void extended_list_scope::SetNoNameIfNameSameAsIdentifier(ref_ptr<value> member, const std::string& identifier)
    {
        auto nameService = m_typeLibrary.GetReflectionTypeLibrary()->GetServiceProvider().GetService<INameService>();
        Objects::Object object(m_typeLibrary.GetReflectionTypeLibrary(), member);
        if(nameService->GetName(object)==identifier)
            nameService->RenameObject(object, "");
    }

    std::string GetCorrectName(const std::string& name, const Objects::Object& object, const std::shared_ptr<INameService>& nameService)
    {
        return name;
        std::string oldName = nameService->GetName(object);
        if (oldName.empty())
            return name;
        else
            return oldName;
    }
    void extended_list_scope::ConditionallySetName(const std::shared_ptr<INewObjectScope>& newObjectScope, ref_ptr<value> member, const std::string& name)
    {
        Objects::Object object(m_typeLibrary.GetReflectionTypeLibrary(), member);
        if (newObjectScope)
        {
            if (newObjectScope->IsNewObject(object))
            {
                auto nameService = m_typeLibrary.GetReflectionTypeLibrary()->GetServiceProvider().GetService<INameService>();
                std::string storedName = nameService->GetStoredName(object);
                if (storedName != name && !storedName.empty())
                {
                    if (list_scope::hasMember(storedName))
                        list_scope::removeMember(storedName);
                }
                nameService->RenameObject(object, name);
            }
        }
        else {
            auto lvalueService = m_typeLibrary.GetReflectionTypeLibrary()->GetServiceProvider().GetService<ILValueService>();
            if (lvalueService->CountLValues(object) == 1)
            {
                auto nameService = m_typeLibrary.GetReflectionTypeLibrary()->GetServiceProvider().GetService<INameService>();
                std::string newName = GetCorrectName(name, object, nameService);
                std::string storedName = nameService->GetStoredName(object);
                if (storedName != newName && !storedName.empty())
                {
                    if (list_scope::hasMember(storedName))
                        list_scope::removeMember(storedName);
                }
                nameService->RenameObject(object, newName);
            }
        }
    }

    void extended_list_scope::ThrowIfNotValidIdentifier(const std::string& identifier)
    {
        if (IsReservedWord(identifier))
            throw std::runtime_error("the name \"" + identifier + "\" is reserved");
    }
}}