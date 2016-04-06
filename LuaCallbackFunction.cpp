#include "LuaCallbackFunction.h"

LuaCallbackFunction::LuaCallbackFunction(std::string function_, int scriptID_)
    : function(function_), scriptID(scriptID_)
{
}

LuaCallbackFunction::LuaCallbackFunction(const LuaCallbackFunction& o)
    : function(o.function), scriptID(o.scriptID)
{
}

LuaCallbackFunction::LuaCallbackFunction()
    : function(""), scriptID(-1)
{
}

