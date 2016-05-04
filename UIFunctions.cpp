#include "UIFunctions.h"
#include "UIProxy.h"
#include "widgets/Event.h"
#include "widgets/Widget.h"

#include <QThread>

#include <iostream>

#include "stubs.h"

UIFunctions *UIFunctions::instance = NULL;

UIFunctions::UIFunctions(QObject *parent)
    : QObject(parent)
{
    UIProxy *uiproxy = UIProxy::getInstance();
    connect(this, SIGNAL(create(Proxy*,Window*)), uiproxy, SLOT(onCreate(Proxy*,Window*)));
    connect(uiproxy, SIGNAL(buttonClick(int)), this, SLOT(onButtonClick(int)));
    connect(uiproxy, SIGNAL(valueChange(int,int)), this, SLOT(onValueChange(int,int)));
    connect(uiproxy, SIGNAL(valueChange(int,QString)), this, SLOT(onValueChange(int,QString)));
    connect(this, SIGNAL(destroy(Proxy*)), uiproxy, SLOT(onDestroy(Proxy*)));
    connect(this, SIGNAL(destroyUi(Window*)), uiproxy, SLOT(onDestroyUi(Window*)));
    connect(this, SIGNAL(showWindow(Window*)), uiproxy, SLOT(onShowWindow(Window*)));
    connect(this, SIGNAL(hideWindow(Window*)), uiproxy, SLOT(onHideWindow(Window*)));
}

UIFunctions::~UIFunctions()
{
    UIFunctions::instance = NULL;
}

UIFunctions * UIFunctions::getInstance(QObject *parent)
{
    if(!UIFunctions::instance)
        UIFunctions::instance = new UIFunctions(parent);
    return UIFunctions::instance;
}

void UIFunctions::destroyInstance()
{
    if(UIFunctions::instance)
        delete UIFunctions::instance;
}

void UIFunctions::onButtonClick(int id)
{
    Widget *widget = Widget::byId(id);
    if(!widget) return;
    EventOnClick *e = dynamic_cast<EventOnClick*>(widget);
    if(e && e->onclick != "" && widget->proxy->scriptID != -1)
    {
        onclickCallback_in in_args;
        in_args.id = id;
        onclickCallback_out out_args;
        onclickCallback(widget->proxy->scriptID, e->onclick.c_str(), &in_args, &out_args);
    }
}

void UIFunctions::onValueChange(int id, int value)
{
    Widget *widget = Widget::byId(id);
    if(!widget) return;
    EventOnChangeInt *e = dynamic_cast<EventOnChangeInt*>(widget);
    if(e && e->onchange != "" && widget->proxy->scriptID != -1)
    {
        onchangeIntCallback_in in_args;
        in_args.id = id;
        in_args.value = value;
        onchangeIntCallback_out out_args;
        onchangeIntCallback(widget->proxy->scriptID, e->onchange.c_str(), &in_args, &out_args);
    }
}

void UIFunctions::onValueChange(int id, QString value)
{
    Widget *widget = Widget::byId(id);
    if(!widget) return;
    EventOnChangeString *e = dynamic_cast<EventOnChangeString*>(widget);
    if(e && e->onchange != "" && widget->proxy->scriptID != -1)
    {
        onchangeStringCallback_in in_args;
        in_args.id = id;
        in_args.value = value.toStdString();
        onchangeStringCallback_out out_args;
        onchangeStringCallback(widget->proxy->scriptID, e->onchange.c_str(), &in_args, &out_args);
    }
}

