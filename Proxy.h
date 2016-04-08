#ifndef PROXY_H_INCLUDED
#define PROXY_H_INCLUDED

#include <map>
#include <set>

#include <QWidget>

#include "UIModel.h"
#include "LuaCallbackFunction.h"

class UIProxy;

class Proxy
{
public:
    Proxy(bool destroyAfterSimulationStop, int scriptID);
    virtual ~Proxy();

    static void destroyTransientObjects();

    inline int getHandle() {return handle;}
    static Proxy* byHandle(int handle);

    void createQtWidget(UIProxy *uiproxy, Window *window);

private:
    // internal handle of this object (used by the plugin):
    int handle;

    static std::map<int, Proxy *> proxies; // handle -> Proxy

    static int nextProxyHandle;

    // objects created during simulation will be destroyed when simulation terminates:
    bool destroyAfterSimulationStop;

    // UIModel's pointer:
    Window *ui;

    // the scriptID from which this object has been created:
    int scriptID;

    friend class UIFunctions;
    friend class UIProxy;
};

#endif // PROXY_H_INCLUDED

