#include "Proxy.h"

#include <vector>
#include <map>
#include <iostream>

#include "v_repLib.h"

int Proxy::nextProxyHandle = 1000;
std::map<int, Proxy *> Proxy::proxies;

Proxy::Proxy(bool destroyAfterSimulationStop_, int scriptID_)
    : handle(nextProxyHandle++),
      destroyAfterSimulationStop(destroyAfterSimulationStop_),
      scriptID(scriptID_),
      ui(NULL)
{
#ifdef DEBUG
    std::cerr << "Proxy::Proxy() - Proxy::proxies[" << handle << "] = " << std::hex << ((void*)this) << std::dec << std::endl;
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
    std::cerr << "Proxy::byHandle(" << handle << ") -> " << std::hex << ret << std::dec << std::endl;
#endif

    return ret;
}

void Proxy::createQtWidget(UIProxy *uiproxy, Window *window)
{
    // this should be executed in the UI thread

    QWidget *mainWindow = (QWidget *)simGetMainWindow(1);
    ui = window;
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
        std::cerr << "    " << it->first << ": " << std::hex << ((void*)it->second) << std::dec << std::endl;
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
        delete Proxy::proxies[*it];
        Proxy::proxies.erase(*it);
    }
}

