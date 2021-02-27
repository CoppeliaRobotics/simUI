#ifndef PROXY_H_INCLUDED
#define PROXY_H_INCLUDED

#include "config.h"

#include <map>
#include <set>

#include <QWidget>

#include "widgets/Window.h"

class UI;

class Proxy
{
public:
    Proxy(bool destroyAfterSimulationStop, int sceneID, int scriptID, int scriptType, Window *window, std::map<int, Widget*>& widgets);
    virtual ~Proxy();

    static void destroyTransientObjects();
    static void destroyAllObjects();
    static void destroyAllObjectsFromUIThread();

    inline int getHandle() {return handle;}
    static Proxy* byHandle(int handle);

    Widget * getWidgetById(int id);

    void createQtWidget(UI *ui);

    inline Window * getWidget() {return window;}

    inline int getScriptID() {return scriptID;}
    inline int getScriptType() {return scriptType;}
    inline int getSceneID() {return sceneID;}

    void sceneChange(int oldSceneID, int newSceneID, void *dummy);
    static void sceneChange(int oldSceneID, int newSceneID);

private:
    // internal handle of this object (used by the plugin):
    int handle;

    static std::map<int, Proxy *> proxies; // handle -> Proxy

    std::map<int, Widget *> widgets; // widgetId -> Widget

    static int nextProxyHandle;

    // objects created during simulation will be destroyed when simulation terminates:
    bool destroyAfterSimulationStop;

    // UIModel's pointer:
    Window *window;

    // the scene from which this object has been created:
    int sceneID;

    // the scriptID from which this object has been created:
    int scriptID;

    // the type of the scriptID above:
    int scriptType;

    friend class SIM;
    friend class UI;
    friend class Widget;
};

#endif // PROXY_H_INCLUDED

