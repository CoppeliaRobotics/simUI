#include "Proxy.h"

#include <vector>
#include <map>
#include <iostream>

#include "UI.h"
#include "SIM.h"

#include <simPlusPlus/Lib.h>

int Proxy::nextProxyHandle = 1000;
std::map<int, Proxy *> Proxy::proxies;

Proxy::Proxy(bool destroyAfterSimulationStop_, int sceneID_, int scriptID_, Window *window_, std::map<int, Widget*>& widgets_)
    : handle(nextProxyHandle++),
      widgets(widgets_),
      destroyAfterSimulationStop(destroyAfterSimulationStop_),
      window(window_),
      sceneID(sceneID_),
      scriptID(scriptID_)
{
    TRACE_FUNC;

    Proxy::proxies[handle] = this;

    sim::addLog(sim_verbosity_debug, "Proxy::proxies[%d] = %x (tableSize=%d)", handle, this, Proxy::proxies.size());
}

Proxy::~Proxy()
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    // should be destroyed from the UI thread

    if(window)
    {
        sim::addLog(sim_verbosity_debug, "delete 'window' member...");

        delete window;
    }

    Proxy::proxies.erase(handle);
}

Proxy* Proxy::byHandle(int handle)
{
    std::map<int, Proxy*>::const_iterator it = Proxy::proxies.find(handle);
    Proxy *ret = it == Proxy::proxies.end() ? NULL : it->second;

    sim::addLog(sim_verbosity_debug, "handle %d -> %x (tableSize=%d)", handle, ret, Proxy::proxies.size());

    return ret;
}

Widget * Proxy::getWidgetById(int id)
{
    std::map<int, Widget*>::const_iterator it = widgets.find(id);
    Widget *ret = it == widgets.end() ? NULL : it->second;
    return ret;
}

void Proxy::createQtWidget(UI *ui)
{
    ASSERT_THREAD(UI);

    window->createQtWidget(this, ui, UI::simMainWindow);
}

// this function will be called at simulation end to destroy objects that
// were created during simulation, which otherwise would leak indefinitely:
void Proxy::destroyTransientObjects()
{
    ASSERT_THREAD(!UI);
    TRACE_FUNC;

    std::vector<int> t;

    for(std::map<int, Proxy*>::const_iterator it = Proxy::proxies.begin(); it != Proxy::proxies.end(); ++it)
    {
        if(it->second->destroyAfterSimulationStop)
            t.push_back(it->first);
    }

    for(std::vector<int>::const_iterator it = t.begin(); it != t.end(); ++it)
    {
        Proxy *proxy = Proxy::byHandle(*it);
        if(proxy)
        {
            sim::addLog(sim_verbosity_debug, "destroying proxy %d... (call SIM::destroy())", proxy->getHandle());
            SIM::getInstance()->destroy(proxy); // will also delete proxy
        }
    }
}

// destroy all objects (must be called from SIM thread):

void Proxy::destroyAllObjects()
{
    ASSERT_THREAD(!UI);
    TRACE_FUNC;

    for(std::map<int, Proxy*>::const_iterator it = Proxy::proxies.begin(); it != Proxy::proxies.end(); ++it)
    {
        Proxy *proxy = it->second;
        if(proxy)
        {
            sim::addLog(sim_verbosity_debug, "destroying proxy %d... (call SIM::destroy())", proxy->getHandle());
            SIM::getInstance()->destroy(proxy); // will also delete proxy
        }
    }
}

// analogous of previous function, but to be used when called from UI thread:

void Proxy::destroyAllObjectsFromUIThread()
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    for(std::map<int, Proxy*>::const_iterator it = Proxy::proxies.begin(); it != Proxy::proxies.end(); ++it)
    {
        Proxy *proxy = it->second;
        if(proxy)
        {
            sim::addLog(sim_verbosity_debug, "destroying proxy %d... (call UI::onDestroy())", proxy->getHandle());
            UI::getInstance()->onDestroy(proxy); // will also delete proxy
        }
    }
}

void Proxy::sceneChange(int oldSceneID, int newSceneID, void *dummy)
{
    ASSERT_THREAD(!UI);
    if(window)
    {
        SIM::getInstance()->sceneChange(window, oldSceneID, newSceneID);
    }
}

void Proxy::sceneChange(int oldSceneID, int newSceneID)
{
    ASSERT_THREAD(!UI);
    void *arg = 0;

    for(std::map<int, Proxy*>::const_iterator it = Proxy::proxies.begin(); it != Proxy::proxies.end(); ++it)
    {
        Proxy *proxy = it->second;
        if(proxy)
            proxy->sceneChange(oldSceneID, newSceneID, arg);
    }
}

