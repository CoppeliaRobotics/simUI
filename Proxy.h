#ifndef PROXY_H_INCLUDED
#define PROXY_H_INCLUDED

#include <map>
#include <set>

#include <QWidget>

#include "LuaCallbackFunction.h"

struct Proxy
{
    // internal handle of this object (used by the plugin):
    int handle;

    // objects created during simulation will be destroyed when simulation terminates:
    bool destroyAfterSimulationStop;

    // widget pointer:
    QWidget *widget;

    std::set<int> widgetIDs;

    void registerOnClickCallback(int widgetID, LuaCallbackFunction cb);
    void registerOnChangeCallback(int widgetID, LuaCallbackFunction cb);

    // event handlers:
    static std::map<int, LuaCallbackFunction> onclickCallback; // widget id -> LuaCallbackFunction
    static std::map<int, LuaCallbackFunction> onchangeCallback; // widget id -> LuaCallbackFunction;
    
    Proxy(bool destroyAfterSimulationStop);
    virtual ~Proxy();

    static std::map<int, Proxy *> proxies; // handle -> Proxy
    static int nextProxyHandle;

    static void destroyTransientObjects();
};

#endif // PROXY_H_INCLUDED

