#include "Proxy.h"

#include <vector>
#include <map>
#include <iostream>

#include "UIFunctions.h"

#include "v_repLib.h"

int Proxy::nextProxyHandle = 1000;
std::map<int, Proxy *> Proxy::proxies;

Proxy::Proxy(bool destroyAfterSimulationStop_, int scriptID_, Window *ui_)
    : handle(nextProxyHandle++),
      destroyAfterSimulationStop(destroyAfterSimulationStop_),
      scriptID(scriptID_),
      ui(ui_)
{
#ifdef DEBUG
    std::cerr << "Proxy::Proxy() - Proxy::proxies[" << handle << "] = " << this << std::endl;
#endif

    Proxy::proxies[handle] = this;
}

Proxy::~Proxy()
{
#ifdef DEBUG
    std::cerr << "Proxy::~Proxy() - begin..." << std::endl;
#endif

    // should be destroyed from the UI thread

    Proxy::proxies.erase(handle);

    if(ui)
    {
#ifdef DEBUG
        std::cerr << "Proxy::~Proxy() - delete member 'ui'" << std::endl;
#endif

        delete ui;
    }

#ifdef DEBUG
    std::cerr << "Proxy::~Proxy() - end" << std::endl;
#endif
}

Proxy* Proxy::byHandle(int handle)
{
    std::map<int, Proxy*>::const_iterator it = Proxy::proxies.find(handle);
    Proxy *ret = it == Proxy::proxies.end() ? NULL : it->second;

#ifdef DEBUG
    std::cerr << "Proxy::byHandle(" << handle << ") -> " << ret << std::endl;
#endif

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
#ifdef DEBUG
    std::cerr << "Proxy::destroyTransientObjects() - content of Proxy::proxies:" << std::endl;

    for(std::map<int, Proxy*>::const_iterator it = Proxy::proxies.begin(); it != Proxy::proxies.end(); ++it)
    {
        std::cerr << "    " << it->first << ": " << it->second << std::endl;
    }
    
    std::cerr << "Proxy::destroyTransientObjects() - end" << std::endl;
#endif

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

