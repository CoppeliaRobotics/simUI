#ifndef LUA_CALLBACK_FUNCTION_H_INCLUDED
#define LUA_CALLBACK_FUNCTION_H_INCLUDED

#include <string>

struct LuaCallbackFunction
{
    // name of the Lua function
    std::string function;
    // id of the V-REP script where the function is defined in
    int scriptID;

    LuaCallbackFunction(std::string function, int scriptID);
    LuaCallbackFunction(const LuaCallbackFunction& o);
    LuaCallbackFunction();
};

#endif // LUA_CALLBACK_FUNCTION_H_INCLUDED

