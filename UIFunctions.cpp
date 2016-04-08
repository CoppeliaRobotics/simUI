#include "UIFunctions.h"

#include <QThread>

#include <iostream>

#include "stubs.h"

UIFunctions::UIFunctions(QObject *parent)
    : QObject(parent)
{
}

UIFunctions::~UIFunctions()
{
}

void UIFunctions::connectToProxy(UIProxy *uiproxy)
{
    connect(this, SIGNAL(create(Proxy*,Window*)), uiproxy, SLOT(onCreate(Proxy*,Window*)));
    connect(uiproxy, SIGNAL(buttonClick(int)), this, SLOT(onButtonClick(int)));
    connect(uiproxy, SIGNAL(valueChange(int,int)), this, SLOT(onValueChange(int,int)));
    connect(uiproxy, SIGNAL(valueChange(int,QString)), this, SLOT(onValueChange(int,QString)));
    connect(this, SIGNAL(destroy(Proxy*)), uiproxy, SLOT(onDestroy(Proxy*)));
}

void UIFunctions::onButtonClick(int id)
{
    Widget *widget = Widget::byId(id);
    if(!widget) return;

    std::string fn = "";
    int scriptID = -1;

    // try to cast to any object that has an 'onclick' member:
    {
        Button *button = dynamic_cast<Button*>(widget);
        if(scriptID == -1 && button)
        {
            fn = button->onclick;
            scriptID = button->proxy->scriptID;
        }
    }
    {
        Radiobutton *radiobutton = dynamic_cast<Radiobutton*>(widget);
        if(scriptID == -1 && radiobutton)
        {
            fn = radiobutton->onclick;
            scriptID = radiobutton->proxy->scriptID;
        }
    }

    if(fn != "" && scriptID != -1)
    {
        onclickCallback_in in_args;
        in_args.id = id;
        onclickCallback_out out_args;
        onclickCallback(scriptID, fn.c_str(), &in_args, &out_args);
    }
}

void UIFunctions::onValueChange(int id, int value)
{
    Widget *widget = Widget::byId(id);
    if(!widget) return;

    std::string fn = "";
    int scriptID = -1;

    // try to cast to any object that has an 'onchange' member:
    HSlider *hslider = dynamic_cast<HSlider*>(widget);
    if(scriptID == -1 && hslider)
    {
        fn = hslider->onchange;
        scriptID = hslider->proxy->scriptID;
    }
    VSlider *vslider = dynamic_cast<VSlider*>(widget);
    if(scriptID == -1 && vslider)
    {
        fn = vslider->onchange;
        scriptID = vslider->proxy->scriptID;
    }
    Checkbox *checkbox = dynamic_cast<Checkbox*>(widget);
    if(scriptID == -1 && checkbox)
    {
        fn = checkbox->onchange;
        scriptID = checkbox->proxy->scriptID;
    }

    if(fn != "" && scriptID != -1)
    {
        onchangeIntCallback_in in_args;
        in_args.id = id;
        in_args.value = value;
        onchangeIntCallback_out out_args;
        onchangeIntCallback(scriptID, fn.c_str(), &in_args, &out_args);
    }
}

void UIFunctions::onValueChange(int id, QString value)
{
    Widget *widget = Widget::byId(id);
    if(!widget) return;

    std::string fn = "";
    int scriptID = -1;

    // try to cast to any object that has an 'onchange' member:
    Edit *edit = dynamic_cast<Edit*>(widget);
    if(scriptID == -1 && edit)
    {
        fn = edit->onchange;
        scriptID = edit->proxy->scriptID;
    }

    if(fn != "" && scriptID != -1)
    {
        onchangeStringCallback_in in_args;
        in_args.id = id;
        in_args.value = value.toStdString();
        onchangeStringCallback_out out_args;
        onchangeStringCallback(scriptID, fn.c_str(), &in_args, &out_args);
    }
}

