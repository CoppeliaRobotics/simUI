#include "Proxy.h"
#include "debug.h"

#include <vector>
#include <map>
#include <iostream>

#include "UIProxy.h"
#include "UIFunctions.h"

#include "v_repLib.h"

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
    DBG << "[enter]" << std::endl;

    Proxy::proxies[handle] = this;

    DBG << "Proxy::proxies[" << handle << "] = " << this << " (tableSize=" << Proxy::proxies.size() << ")" << std::endl;
    DBG << "[leave]" << std::endl;
}

Proxy::~Proxy()
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    // should be destroyed from the UI thread

    if(ui)
    {
        DBG << "delete 'ui' member..." << std::endl;

        delete ui;
    }

    Proxy::proxies.erase(handle);

    DBG << "[leave]" << std::endl;
}

Proxy* Proxy::byHandle(int handle)
{
    std::map<int, Proxy*>::const_iterator it = Proxy::proxies.find(handle);
    Proxy *ret = it == Proxy::proxies.end() ? NULL : it->second;

    DBG << "handle " << handle << " -> " << ret << " (tableSize=" << Proxy::proxies.size() << ")" << std::endl;

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

    ui->createQtWidget(this, uiproxy, UIProxy::vrepMainWindow);
}

// this function will be called at simulation end to destroy objects that
// were created during simulation, which otherwise would leak indefinitely:
void Proxy::destroyTransientObjects()
{
    ASSERT_THREAD(!UI);
    DBG << "[enter]" << std::endl;

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
            DBG << "destroying proxy " << proxy->getHandle() << "... (call UIFunctions::destroy())" << std::endl;
            UIFunctions::getInstance()->destroy(proxy); // will also delete proxy
        }
    }

    DBG << "[leave]" << std::endl;
}

// destroy all objects (must be called from SIM thread):

void Proxy::destroyAllObjects()
{
    ASSERT_THREAD(!UI);
    DBG << "[enter]" << std::endl;

    for(std::map<int, Proxy*>::const_iterator it = Proxy::proxies.begin(); it != Proxy::proxies.end(); ++it)
    {
        Proxy *proxy = it->second;
        if(proxy)
        {
            DBG << "destroying proxy " << proxy->getHandle() << "... (call UIFunctions::destroy())" << std::endl;
            UIFunctions::getInstance()->destroy(proxy); // will also delete proxy
        }
    }

    DBG << "[leave]" << std::endl;
}

// analogous of previous function, but to be used when called from UI thread:

void Proxy::destroyAllObjectsFromUIThread()
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    for(std::map<int, Proxy*>::const_iterator it = Proxy::proxies.begin(); it != Proxy::proxies.end(); ++it)
    {
        Proxy *proxy = it->second;
        if(proxy)
        {
            DBG << "destroying proxy " << proxy->getHandle() << "... (call UIProxy::onDestroy())" << std::endl;
            UIProxy::getInstance()->onDestroy(proxy); // will also delete proxy
        }
    }

    DBG << "[leave]" << std::endl;
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

