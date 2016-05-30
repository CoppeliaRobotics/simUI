#include "Proxy.h"
#include "debug.h"

#include <vector>
#include <map>
#include <iostream>

#include "UIFunctions.h"

#include "v_repLib.h"

int Proxy::nextProxyHandle = 1000;
std::map<int, Proxy *> Proxy::proxies;

Proxy::Proxy(bool destroyAfterSimulationStop_, int sceneID_, int scriptID_, Window *ui_, std::map<int, Widget*>& widgets_)
    : handle(nextProxyHandle++),
      destroying(false),
      destroyAfterSimulationStop(destroyAfterSimulationStop_),
      sceneID(sceneID_),
      scriptID(scriptID_),
      ui(ui_),
      widgets(widgets_)
{
    DBG << "Proxy::proxies[" << handle << "] = " << this << std::endl;

    Proxy::proxies[handle] = this;
}

Proxy::~Proxy()
{
    DBG << "begin..." << std::endl;

    destroying = true;

    // should be destroyed from the UI thread

    Proxy::proxies.erase(handle);

    if(ui)
    {
        DBG << "delete member 'ui'..." << std::endl;
        delete ui;
    }

    DBG << "end" << std::endl;
}

Proxy* Proxy::byHandle(int handle)
{
    std::map<int, Proxy*>::const_iterator it = Proxy::proxies.find(handle);
    Proxy *ret = it == Proxy::proxies.end() ? NULL : it->second;

    DBG << "handle " << handle << " -> " << ret << std::endl;

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
    // this should be executed in the UI thread

    QWidget *mainWindow = (QWidget *)simGetMainWindow(1);
    ui->createQtWidget(this, uiproxy, mainWindow);
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
        Proxy *proxy = Proxy::byHandle(*it);
        if(proxy)
            UIFunctions::getInstance()->destroy(proxy); // will also delete proxy
    }
}

void Proxy::destroyAllObjects()
{
    for(std::map<int, Proxy*>::const_iterator it = Proxy::proxies.begin(); it != Proxy::proxies.end(); ++it)
    {
        Proxy *proxy = it->second;
        if(proxy)
            UIFunctions::getInstance()->destroy(proxy); // will also delete proxy
    }
}

void Proxy::sceneChange(int oldSceneID, int newSceneID, void *dummy)
{
    if(ui)
    {
        UIFunctions::getInstance()->sceneChange(ui, oldSceneID, newSceneID);
    }
}

void Proxy::sceneChange(int oldSceneID, int newSceneID)
{
    void *arg = 0;

    for(std::map<int, Proxy*>::const_iterator it = Proxy::proxies.begin(); it != Proxy::proxies.end(); ++it)
    {
        Proxy *proxy = it->second;
        if(proxy)
            proxy->sceneChange(oldSceneID, newSceneID, arg);
    }
}

