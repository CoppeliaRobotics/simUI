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
    connect(this, SIGNAL(create(Proxy*)), uiproxy, SLOT(onCreate(Proxy*)));
    connect(uiproxy, SIGNAL(buttonClick(Widget*)), this, SLOT(onButtonClick(Widget*)));
    connect(uiproxy, SIGNAL(valueChange(Widget*,int)), this, SLOT(onValueChange(Widget*,int)));
    connect(uiproxy, SIGNAL(valueChange(Widget*,QString)), this, SLOT(onValueChange(Widget*,QString)));
    connect(uiproxy, SIGNAL(windowClose(Window*)), this, SLOT(onWindowClose(Window*)));
    connect(this, SIGNAL(destroy(Proxy*)), uiproxy, SLOT(onDestroy(Proxy*)));
    connect(this, SIGNAL(destroyUi(Window*)), uiproxy, SLOT(onDestroyUi(Window*)));
    connect(this, SIGNAL(showWindow(Window*)), uiproxy, SLOT(onShowWindow(Window*)));
    connect(this, SIGNAL(hideWindow(Window*)), uiproxy, SLOT(onHideWindow(Window*)));
    connect(this, SIGNAL(setImage(Image*,const char*,int,int)), uiproxy, SLOT(onSetImage(Image*,const char*,int,int)));
    connect(uiproxy, SIGNAL(loadImageFromFile(Image*,const char *,int,int)), this, SLOT(onLoadImageFromFile(Image*,const char *,int,int)));
    connect(this, SIGNAL(sceneChange(Window*,int,int)), uiproxy, SLOT(onSceneChange(Window*,int,int)));
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

void UIFunctions::onButtonClick(Widget *widget)
{
    EventOnClick *e = dynamic_cast<EventOnClick*>(widget);
    if(e && e->onclick != "" && widget->proxy->scriptID != -1)
    {
        onclickCallback_in in_args;
        in_args.handle = widget->proxy->handle;
        in_args.id = widget->id;
        onclickCallback_out out_args;
        onclickCallback(widget->proxy->scriptID, e->onclick.c_str(), &in_args, &out_args);
    }
}

void UIFunctions::onValueChange(Widget *widget, int value)
{
    EventOnChangeInt *e = dynamic_cast<EventOnChangeInt*>(widget);
    if(e && e->onchange != "" && widget->proxy->scriptID != -1)
    {
        onchangeIntCallback_in in_args;
        in_args.handle = widget->proxy->handle;
        in_args.id = widget->id;
        in_args.value = value;
        onchangeIntCallback_out out_args;
        onchangeIntCallback(widget->proxy->scriptID, e->onchange.c_str(), &in_args, &out_args);
    }
}

void UIFunctions::onValueChange(Widget *widget, QString value)
{
    EventOnChangeString *e = dynamic_cast<EventOnChangeString*>(widget);
    if(e && e->onchange != "" && widget->proxy->scriptID != -1)
    {
        onchangeStringCallback_in in_args;
        in_args.handle = widget->proxy->handle;
        in_args.id = widget->id;
        in_args.value = value.toStdString();
        onchangeStringCallback_out out_args;
        onchangeStringCallback(widget->proxy->scriptID, e->onchange.c_str(), &in_args, &out_args);
    }
}

void UIFunctions::onWindowClose(Window *window)
{
    oncloseCallback_in in_args;
    in_args.handle = window->proxy->getHandle();
    oncloseCallback_out out_args;
    oncloseCallback(window->proxy->getScriptID(), window->onclose.c_str(), &in_args, &out_args);
}

void UIFunctions::onLoadImageFromFile(Image *image, const char *filename, int w, int h)
{
    QImage::Format format = QImage::Format_RGB888;
    int bpp = 3; // bytes per pixel
    int resolution[2];
    simUChar *data = simLoadImage(resolution, 0, filename, NULL);
    simTransformImage(data, resolution, 4, NULL, NULL, NULL);

    if(w > 0 && h > 0)
    {
        int size[2] = {w, h};
        simUChar *scaled = simGetScaledImage(data, resolution, size, 0, NULL);
        simReleaseBufferE((simChar *)data);
        setImage(image, (simChar *)scaled, w, h);
    }
    else
    {
        setImage(image, (simChar *)data, resolution[0], resolution[1]);
    }
}

