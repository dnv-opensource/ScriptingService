#pragma once
#include <string>
#include <iostream>
#include "jsScript/jsClass.h"
#include "jsScript/jsStack.h"
#include "jsScript/jsModelObject.h"
inline void Print(const std::string& name)
{
    std::cout << name << std::endl;
}

inline void RegisterPrint()
{
    jsTClass<jsModelObject> cls(jsStack::stack()->GetJsTypeLibrary());
    cls.Function("Print", Print);
}