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
#include "jsScriptingService.h"
#include "ixlib_javascript.hh"
#include "Ixion\extended_list_scope.h"
#include "Ixion\ReflectionValueCreatorService.h"

#include "Reflection\Objects\Object.h"
#include "Services\Allocators\ForwardingAllocatorSelector.h"
#include "jsScript\jsTypeLibrary.h"
#include "Reflection\NativeTypes.Reflection.h"
#include "Reflection\Reflect.h"
#include "Reflection\Classes\Class.h"
#include "jsScriptingServiceStack.h"
#include "jsScript\jsStack.h"
#include "Scripting\jsTypesRegistration.h"
#include "ixlib_js_internals.hh"
#include "jsUnitParser\Units\UnitParserCaller.h"
#include "jsUnitParser\Units\Runtime\DynamicUnitParser.h"
#include "Ixion\ReflectionUnitHelper.h"
#include "Ixion\ReflectIxion.h"
#include "Ixion\ref_ptr.Reflection.h"
#include "jsScriptingLValueService.h"
#include "Services\Allocators\PersistenceControl.h"
#include "TypeUtilities\ScopedVariable.h"
#include "jsScript\AutoCompletion\jsAutoCompletionService.h"
#include "jsScript\AutoCompletion\CompositeAutoCompletionContext.h"
#include "jsScript\AutoCompletion\jsAutoCompletionContext.h"
#include "jsScript\AutoCompletion\ReflectionAutoCompletionContext.h"
#include "ReflectionFunctionWrapper.h"
#include "Ixion\ConversionHelper.h"
#include "Services\ServiceProvider.h"
#include "Services\Allocators\IAllocatorSelectorService.h"
#include "Reflection\LValue.h"
#include "Ixion\reflected_value.h"
#include "jsScriptingPropertyService.h"
#include "Reflection\Variants\VariantService.h"
#include "jsScript\jsExceptions.h"
#include <iosfwd>
#include "Ixion\IxionAutoCompletionContext.h"
#include "jsScript\jsScopedDummyMode.h"
#include "jsScript\jsDummyTypeLibrary.h"
#include "Reflection\TypeLibraries\TypeLibrary.h"

using namespace DNVS::MoFa::Reflection;
using namespace DNVS::MoFa::Services;
using namespace DNVS::MoFa::Scripting;
using namespace DNVS::MoFa::TypeUtilities;

class jsScriptingServiceImpl {
public:
    jsScriptingServiceImpl(jsTypeLibrary& typeLibrary)
        : jsScriptingServiceImpl(typeLibrary, std::make_shared<ixion::javascript::ReflectionValueCreatorService>(typeLibrary.GetReflectionTypeLibrary()))
    {
        using namespace DNVS::MoFa::Units::Parser;
        ixion::javascript::setUnitParser(
            UnitParserCaller<DynamicUnitParser>(std::make_shared<ReflectionUnitHelper>(typeLibrary.GetReflectionTypeLibrary()))
        );
    }
    ~jsScriptingServiceImpl() {
            ixion::javascript::setUnitParser(nullptr);
    }
    ixion::javascript::interpreter interpreter;
    ixion::javascript::extended_list_scope* extendedListScope;
    jsScriptingServiceStack m_stack;
private:
    jsScriptingServiceImpl(jsTypeLibrary& typeLibrary, const std::shared_ptr<ixion::javascript::IValueCreatorService>& valueCreatorService)
        :   jsScriptingServiceImpl(typeLibrary, valueCreatorService, new ixion::javascript::extended_list_scope(typeLibrary, valueCreatorService))
    {}
    jsScriptingServiceImpl(jsTypeLibrary& typeLibrary, const std::shared_ptr<ixion::javascript::IValueCreatorService>& valueCreatorService, ixion::javascript::extended_list_scope* listScope)
        : interpreter(listScope, valueCreatorService)
        , extendedListScope(listScope)
        , m_stack(typeLibrary)
    {
        ixion::javascript::ReflectIxion(typeLibrary);       
        ReflectNative(typeLibrary.GetReflectionTypeLibrary());
        RegisterJsTypes(typeLibrary);
        using namespace Classes;
        Class<Members::GlobalType> global(typeLibrary.GetReflectionTypeLibrary(), "");
        global.StaticFunction("eval",[&](const std::string& expression) 
        {
            return Objects::Object(extendedListScope->GetReflectionTypeLibrary(), interpreter.execute(expression));
        }).AddSignature("expression");
    }
};

jsScriptingService::jsScriptingService(const std::shared_ptr<jsTypeLibrary>& typeLibrary, std::shared_ptr<INameService> nameService, const std::function<void(int)>& lineNumberCallback, const std::function<void(int)>& lastLineNumberCallback)
    : m_typeLibrary(typeLibrary)
    , m_impl(new jsScriptingServiceImpl(*typeLibrary))
    , m_executionStatus(ExecutionStatus::NoExecution)
{
    if(lineNumberCallback)
        m_impl->extendedListScope->SetLineNumberHandler(lineNumberCallback);
    if(lastLineNumberCallback)
        m_impl->extendedListScope->SetLastLineNumberHandler(lastLineNumberCallback);
    typeLibrary->GetReflectionTypeLibrary()->GetServiceProvider().RegisterService<ILValueService>(std::make_shared<jsScriptingLValueService>());
    typeLibrary->GetReflectionTypeLibrary()->GetServiceProvider().RegisterService(nameService);
    ServiceProvider::Instance().RegisterService<IScriptingPropertyService>(std::make_shared<jsScriptingPropertyService>());
}

jsScriptingService::~jsScriptingService()
{
    m_impl->extendedListScope->clearMembers();    
    m_typeLibrary->GetReflectionTypeLibrary()->GetServiceProvider().UnregisterService<ILValueService>();
    m_typeLibrary->GetReflectionTypeLibrary()->GetServiceProvider().UnregisterService<INameService>();
}

template<typename Lambda>
Objects::Object TryExecute(Lambda lambda)
{
    try
    {
        return lambda();
    }
    catch (ixion::javascript_exception& e)
    {
        throw line_exception(e.what(), (int)e.Line);
    }
    catch (std::exception& e)
    {
        std::string line = e.what();
        std::string::size_type where = line.find("line: ");

        if (where == std::string::npos)
            throw;

        where += 6;
        std::istringstream stream(&line[where]);
        int iline;
        stream >> iline;

        throw line_exception(e.what(), iline);
    }
    catch (...)
    {
        throw std::runtime_error("Unknown scripting error");
    }
}
Objects::Object jsScriptingService::Execute(const std::string& str, bool logging /*= false*/)
{
    return TryExecute([&]()
    {
        jsStackPusher pusher;
        Allocators::PersistenceControl control(true);
        auto status = Scope<ExecutionStatus>(m_executionStatus, ExecutionStatus::ExecuteString);
        return Objects::Object(m_typeLibrary->GetReflectionTypeLibrary(), m_impl->interpreter.execute(str));
    });
}

Objects::Object jsScriptingService::Execute(std::istream& istr, bool logging /*= false*/)
{
    return TryExecute([&]()
    {
        jsStackPusher pusher;
        Allocators::PersistenceControl control(true);
        auto status = Scope<ExecutionStatus>(m_executionStatus, ExecutionStatus::ExecuteFile);
        return Objects::Object(m_typeLibrary->GetReflectionTypeLibrary(), m_impl->interpreter.execute(istr));
    });
}

Objects::Object jsScriptingService::Test(const std::string& str)
{
    std::string testExpression = str;
    if (testExpression.length() > 2)
    {
        if (testExpression.substr(testExpression.length() - 2) == "};")
            testExpression = testExpression.substr(0, testExpression.length() - 1);
    }
    return TryExecute([&]()
    {
        jsStackPusher pusher;
        Allocators::PersistenceControl control(false);
        jsScopedDummyMode dummyMode;
        return Objects::Object(std::make_shared<jsDummyTypeLibrary>(m_typeLibrary->GetReflectionTypeLibrary()), m_impl->interpreter.execute(testExpression));
    });
}

Objects::Object jsScriptingService::Invoke(const std::string& expression, const std::vector<Objects::Object>& arguments)
{
    std::string testExpression = expression;
    if (testExpression.length() > 2)
    {
        if (testExpression.substr(testExpression.length() - 2) == "};")
            testExpression = testExpression.substr(0, testExpression.length() - 1);
    }
    return TryExecute([&]()
    {
        jsStackPusher pusher;
        Allocators::PersistenceControl control(false);
        auto dummyMode = DNVS::MoFa::TypeUtilities::Scope<bool>(
            []() {return jsStack::stack()->dummyMode(); },
            [](bool val) {jsStack::stack()->setDummyMode(val); },
            true);
        using namespace ixion;
        using namespace ixion::javascript;
        ref_ptr<value> result = m_impl->interpreter.execute(testExpression);
        if (!result)
            return Objects::Object();
        value::parameter_list parameters(arguments.size());
        for (size_t i = 0; i < arguments.size(); ++i)
        {
            parameters[i] = ConversionHelper::ToIxion(arguments[i]);
        }
        return Objects::Object(m_typeLibrary->GetReflectionTypeLibrary(), result->call(parameters));
    });
}

TypeConversions::ConversionGraphPointer jsScriptingService::GetConversionGraph() const
{
    return m_typeLibrary->GetReflectionTypeLibrary()->GetConversionGraph();
}

TypeLibraries::TypeLibraryPointer jsScriptingService::GetTypeLibrary() const
{
    return m_typeLibrary->GetReflectionTypeLibrary();
}

Allocators::ForwardingAllocatorSelector jsScriptingService::GetAllocator() const
{
    auto service = ServiceProvider::Instance().TryGetService<Allocators::IAllocatorSelectorService>();
    if (service)
        return Allocators::ForwardingAllocatorSelector(service->GetAllocatorSelector());
    else
        return Allocators::ForwardingAllocatorSelector();
}

bool jsScriptingService::IsLookupExpression(const std::string& expr) const
{
    try {
        auto expression = m_impl->interpreter.parse(expr);
        if (expression)
            return expression->IsLookupExpression();
        else
            return false;
    }
    catch (...)
    {
        return false;
    }
}

bool jsScriptingService::TrySplitFunctionArguments(const std::string& expr, std::string& functionName, std::vector<std::string>& arguments) const
{
    try {
        auto expression = m_impl->interpreter.parse(expr);
        if (expression)
            return expression->TrySplitFunctionArguments(functionName, arguments);
        else
            return false;
    }
    catch (...)
    {
        return false;
    }
}

bool jsScriptingService::HasIdentifier(const std::string& name) const
{
    try {
        m_impl->extendedListScope->lookup(name);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool jsScriptingService::HasMember(const std::string& name) const
{    
    if (m_typeLibrary->GetReflectionTypeLibrary()->GetServiceProvider().GetService<INameService>()->IsNameInUse(name))
        return true;
    if (m_impl->extendedListScope->hasMember(name))
        return true;
    return false;
}

void jsScriptingService::ClearMembers()
{
    auto nameService = m_typeLibrary->GetReflectionTypeLibrary()->GetServiceProvider().GetService<INameService>();
    auto lvalueService = m_typeLibrary->GetReflectionTypeLibrary()->GetServiceProvider().GetService<ILValueService>();
    for (const std::string& name : nameService->GetAllNames())
    {
        if (m_impl->extendedListScope->hasMember(name))
        {
            m_impl->extendedListScope->removeMember(name);
        }            
    }
    lvalueService->Clear();
    nameService->Clear();
}

std::set<std::string> jsScriptingService::GetAllMemberNames() const
{
    return m_impl->extendedListScope->GetNames();
}

void jsScriptingService::DeleteMember(const std::string& name)
{
    ixion::ref_ptr<ixion::javascript::value> member;
    try {
        member = m_impl->interpreter.RootScope->lookup(name);
    }
    catch(...)
    { }
    m_impl->interpreter.RootScope->removeMember(name);
    auto nameService = m_typeLibrary->GetReflectionTypeLibrary()->GetServiceProvider().GetService<INameService>();
    auto lvalueService = m_typeLibrary->GetReflectionTypeLibrary()->GetServiceProvider().GetService<ILValueService>();
    auto object = nameService->GetObject(name);   
    if (object.IsValid())
    {
        nameService->RemoveObject(object);
        lvalueService->DeleteObject(object);
    }
    if (member)
    {
        auto memberObject = Objects::Object(m_typeLibrary->GetReflectionTypeLibrary(), member);
        if (memberObject.IsValid())
        {
            lvalueService->DeleteObject(memberObject);
        }
    }
}

void jsScriptingService::RenameMember(const Objects::Object& object, const std::string& newName, bool forceRename)
{
    if (forceRename)
    {
        if (auto service = m_typeLibrary->GetReflectionTypeLibrary()->GetServiceProvider().TryGetService<INewObjectScope>())
            service->RenameObject(object, true);
    }
    auto nameService = m_typeLibrary->GetReflectionTypeLibrary()->GetServiceProvider().GetService<INameService>();
    std::string oldName = nameService->GetStoredName(object);
    if (oldName == newName)
    {
        if(!newName.empty() && forceRename && !m_impl->extendedListScope->hasMember(newName))
        {
            auto variant = Variants::VariantService::Reflect(std::make_unique<LValue>(object.ConvertToDynamicType()));
            Objects::Object lvalue(m_typeLibrary->GetReflectionTypeLibrary(), variant);
            m_impl->extendedListScope->addMember(newName, new ixion::javascript::reflected_value(lvalue));
        }
        return;
    }
    if (!oldName.empty() && m_impl->extendedListScope->hasMember(oldName))
    {
        auto member = m_impl->extendedListScope->lookup(oldName);
        m_impl->extendedListScope->removeMember(newName);
        m_impl->extendedListScope->addMember(newName, member);
        nameService->RenameObject(Objects::Object(m_typeLibrary->GetReflectionTypeLibrary(), member), newName);
        m_impl->extendedListScope->removeMember(oldName);
    }
    else
    {
        auto variant = Variants::VariantService::Reflect(std::make_unique<LValue>(object.ConvertToDynamicType()));
        Objects::Object lvalue(m_typeLibrary->GetReflectionTypeLibrary(), variant);
        m_impl->extendedListScope->removeMember(newName);
        m_impl->extendedListScope->addMember(newName, new ixion::javascript::reflected_value(lvalue));
        nameService->RenameObject(lvalue, newName);
    }
}

bool jsScriptingService::InAssignment() const
{
    return m_impl->extendedListScope->IsInAssignment();
}

std::shared_ptr<IAutoCompletion> jsScriptingService::TryAutoComplete(const std::string& text, bool functionCompletion /*= true*/) const
{
    jsStackPusher pusher;
    using namespace DNVS::MoFa::Scripting;
    jsAutoCompletionService service(std::make_shared<CompositeAutoCompletionContext>(
        std::make_shared<jsAutoCompletionContext>(),
        std::make_shared<ReflectionAutoCompletionContext>(m_typeLibrary->GetReflectionTypeLibrary(),std::make_shared<ReflectionFunctionWrapper>(m_typeLibrary->GetReflectionTypeLibrary())),
        std::make_shared<ixion::javascript::IxionAutoCompletionContext>(*m_impl->extendedListScope)
    ));
    return service.TryAutocomplete(text, functionCompletion);
}

ExecutionStatus jsScriptingService::GetExecutionStatus() const
{
    return m_executionStatus;
}

Objects::Object jsScriptingService::Lookup(const std::string& name)
{
    try {
        return Objects::Object(m_typeLibrary->GetReflectionTypeLibrary(), m_impl->extendedListScope->lookup(name));
    }
    catch (...)
    {
        return Objects::Object(m_typeLibrary->GetReflectionTypeLibrary());
    }
}

bool jsScriptingService::TryGetName(const Objects::Object& object, std::string& name)
{
    auto nameService = m_typeLibrary->GetReflectionTypeLibrary()->GetServiceProvider().GetService<INameService>();
    if (nameService->HasName(object))
    {
        name = nameService->GetName(object);
        return true;
    }
    return false;
}

extern "C" __declspec(dllexport) DNVS::MoFa::Scripting::IScriptingService* CreateScriptingService(const std::shared_ptr<jsTypeLibrary>& typeLibrary, const std::shared_ptr<DNVS::MoFa::Scripting::INameService>& nameService, const std::function<void(int)>& lineNumberCallback = nullptr, const std::function<void(int)>& lastLineNumberCallback = nullptr)
{
    return new jsScriptingService(typeLibrary, nameService, lineNumberCallback, lastLineNumberCallback);
}