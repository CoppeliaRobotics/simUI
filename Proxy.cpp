#include "Proxy.h"
#include "debug.h"

#include <vector>
#include <map>
#include <iostream>

#include "UIProxy.h"
#include "UIFunctions.h"

#include "simLib.h"

int Proxy::nextProxyHandle = 1000;
std::map<int, Proxy *> Proxy::proxies;

Proxy::Proxy(bool destroyAfterSimulationStop_, int sceneID_, int scriptID_, Window *ui_, std::map<int, Widget*>& widgets_)
    : handle(nextProxyHandle++),
      widgets(widgets_),
      destroyAfterSimulationStop(destroyAfterSimulationStop_),
      ui(ui_),
      sceneID(sceneID_),
      scriptID(scriptID_)
{
    TRACE_FUNC;

    Proxy::proxies[handle] = this;

    log(sim_verbosity_debug, boost::format("Proxy::proxies[%d] = %x (tableSize=%d)") % handle % this % Proxy::proxies.size());
}

Proxy::~Proxy()
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    // should be destroyed from the UI thread

    if(ui)
    {
        log(sim_verbosity_debug, "delete 'ui' member...");

        delete ui;
    }

    Proxy::proxies.erase(handle);
}

Proxy* Proxy::byHandle(int handle)
{
    std::map<int, Proxy*>::const_iterator it = Proxy::proxies.find(handle);
    Proxy *ret = it == Proxy::proxies.end() ? NULL : it->second;

    log(sim_verbosity_debug, boost::format("handle %d -> %x (tableSize=%d)") % handle % ret % Proxy::proxies.size());

    return ret;
}

Widget * Proxy::getWidgetById(int id)
{
    std::map<int, Widget*>::const_iterator it = widgets.find(id);
    Widget *ret = it == widgets.end() ? NULL : it->second;
    return ret;
}

void Proxy::createQtWidget(UIProxy *uiproxy)
{
    ASSERT_THREAD(UI);

    ui->createQtWidget(this, uiproxy, UIProxy::simMainWindow);
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
            log(sim_verbosity_debug, boost::format("destroying proxy %d... (call UIFunctions::destroy())") % proxy->getHandle());
            UIFunctions::getInstance()->destroy(proxy); // will also delete proxy
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
            log(sim_verbosity_debug, boost::format("destroying proxy %d... (call UIFunctions::destroy())") % proxy->getHandle());
            UIFunctions::getInstance()->destroy(proxy); // will also delete proxy
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
            log(sim_verbosity_debug, boost::format("destroying proxy %d... (call UIProxy::onDestroy())") % proxy->getHandle());
            UIProxy::getInstance()->onDestroy(proxy); // will also delete proxy
        }
    }
}

void Proxy::sceneChange(int oldSceneID, int newSceneID, void *dummy)
{
    ASSERT_THREAD(!UI);
    if(ui)
    {
        UIFunctions::getInstance()->sceneChange(ui, oldSceneID, newSceneID);
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

