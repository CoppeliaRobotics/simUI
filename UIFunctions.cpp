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
    connect(this, SIGNAL(create(Proxy*,int,QString)), uiproxy, SLOT(onCreate(Proxy*,int,QString)));
    connect(uiproxy, SIGNAL(buttonClick(int)), this, SLOT(onButtonClick(int)));
    connect(uiproxy, SIGNAL(valueChange(int,int)), this, SLOT(onValueChange(int,int)));
    connect(uiproxy, SIGNAL(valueChange(int,QString)), this, SLOT(onValueChange(int,QString)));
    connect(this, SIGNAL(destroy(Proxy*)), uiproxy, SLOT(onDestroy(Proxy*)));
}

void UIFunctions::onButtonClick(int id)
{
    try
    {
        LuaCallbackFunction& cb = Proxy::onclickCallback[id];
        onclickCallback_in in_args;
        in_args.id = id;
        onclickCallback_out out_args;
        onclickCallback(cb.scriptID, cb.function.c_str(), &in_args, &out_args);
    }
    catch(std::exception& ex)
    {
    }
}

void UIFunctions::onValueChange(int id, int value)
{
    try
    {
        LuaCallbackFunction& cb = Proxy::onchangeCallback[id];
        onchangeIntCallback_in in_args;
        in_args.id = id;
        in_args.value = value;
        onchangeIntCallback_out out_args;
        onchangeIntCallback(cb.scriptID, cb.function.c_str(), &in_args, &out_args);
    }
    catch(std::exception& ex)
    {
    }
}

void UIFunctions::onValueChange(int id, QString value)
{
    try
    {
        LuaCallbackFunction& cb = Proxy::onchangeCallback[id];
        onchangeStringCallback_in in_args;
        in_args.id = id;
        in_args.value = value.toStdString();
        onchangeStringCallback_out out_args;
        onchangeStringCallback(cb.scriptID, cb.function.c_str(), &in_args, &out_args);
    }
    catch(std::exception& ex)
    {
    }
}

