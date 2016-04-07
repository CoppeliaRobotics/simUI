#include "Proxy.h"

#include <vector>

int Proxy::nextProxyHandle = 1000;
std::map<int, LuaCallbackFunction> Proxy::onclickCallback;
std::map<int, LuaCallbackFunction> Proxy::onchangeCallback;
std::map<int, Proxy *> Proxy::proxies;

Proxy::Proxy(bool destroyAfterSimulationStop_)
    : handle(nextProxyHandle++),
      destroyAfterSimulationStop(destroyAfterSimulationStop_),
      widget(NULL)
{
    Proxy::proxies[handle] = this;
}

Proxy::~Proxy()
{
    Proxy::proxies.erase(handle);

    for(std::set<int>::const_iterator it = widgetIDs.begin(); it != widgetIDs.end(); ++it)
    {
        Proxy::onclickCallback.erase(*it);
        Proxy::onchangeCallback.erase(*it);
    }
}

// this function will be called at simulation end to destroy objects that
// were created during simulation, which otherwise would leak indefinitely:
void Proxy::destroyTransientObjects()
{
    std::vector<int> t;

    for(std::map<int, Proxy*>::const_iterator it = Proxy::proxies.begin(); it != Proxy::proxies.end(); ++it)
    {
        if(it->second->destroyAfterSimulationStop)
            t.push_back(it->first);
    }

    for(std::vector<int>::const_iterator it = t.begin(); it != t.end(); ++it)
    {
        delete Proxy::proxies[*it];
    }
}

void Proxy::registerOnClickCallback(int widgetID, LuaCallbackFunction cb)
{
    widgetIDs.insert(widgetID);
    Proxy::onclickCallback[widgetID] = cb;
}

void Proxy::registerOnChangeCallback(int widgetID, LuaCallbackFunction cb)
{
    widgetIDs.insert(widgetID);
    Proxy::onchangeCallback[widgetID] = cb;
}

