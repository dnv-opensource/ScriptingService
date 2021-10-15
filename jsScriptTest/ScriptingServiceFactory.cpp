#include "ScriptingServiceFactory.h"
#include "Reflection/TypeLibraries/TypeLibraryFactory.h"
#include "jsScript/jsTypeLibrary.h"
#include "Scripting/ReflectionNameService.h"
#include "Reflection/Reflect.h"
#include "Reflection/Reflection/IMember_Reflection.h"
using namespace DNVS::MoFa::Scripting;
using namespace DNVS::MoFa::Reflection;
extern "C" __declspec(dllimport) IScriptingService* CreateScriptingService(const std::shared_ptr<jsTypeLibrary>& typeLibrary, const std::shared_ptr<INameService>& nameService, const std::function<void(int)>& lineNumberCallback = nullptr);



std::shared_ptr<IScriptingService> ScriptingServiceFactory::CreateScriptingService()
{
    auto typeLibrary = TypeLibraries::TypeLibraryFactory::CreateDefaultTypeLibrary(false);
    Reflect<Members::IMember>(typeLibrary);

    return std::shared_ptr<IScriptingService>(::CreateScriptingService(std::make_shared<jsTypeLibrary>(typeLibrary), std::make_shared<ReflectionNameService>()));
}
