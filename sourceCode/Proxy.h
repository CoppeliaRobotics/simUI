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
    Proxy(int sceneID, int scriptID, const std::string &scriptType, Window *window, std::map<int, Widget*>& widgets);
    virtual ~Proxy();

    Widget * getWidgetById(int id);

    void createQtWidget(UI *ui);

    inline Window * getWidget() {return window;}

    inline int getScriptID() {return scriptID;}
    inline std::string getScriptType() {return scriptType;}
    inline int getSceneID() {return sceneID;}

private:
    std::map<int, Widget *> widgets; // widgetId -> Widget

    std::string handle;

    // UIModel's pointer:
    Window *window;

    // the scene from which this object has been created:
    int sceneID;

    // the scriptID from which this object has been created:
    int scriptID;

    // the type of the scriptID above:
    std::string scriptType;

    friend class SIM;
    friend class UI;
    friend class Widget;
    friend class Plugin;
};

#endif // PROXY_H_INCLUDED
