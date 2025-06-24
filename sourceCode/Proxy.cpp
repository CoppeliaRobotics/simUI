#include "Proxy.h"

#include <vector>
#include <map>
#include <iostream>

#include "UI.h"
#include "SIM.h"

#include <simPlusPlus-2/Lib.h>

Proxy::Proxy(int sceneID_, int scriptID_, int scriptType_, Window *window_, std::map<int, Widget*>& widgets_)
    : widgets(widgets_),
      window(window_),
      sceneID(sceneID_),
      scriptID(scriptID_),
      scriptType(scriptType_)
{
    TRACE_FUNC;
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
