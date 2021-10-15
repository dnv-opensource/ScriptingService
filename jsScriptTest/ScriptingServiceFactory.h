#pragma once
#include "Scripting/IScriptingService.h"
class ScriptingServiceFactory {
public:
    std::shared_ptr<DNVS::MoFa::Scripting::IScriptingService> CreateScriptingService();
};