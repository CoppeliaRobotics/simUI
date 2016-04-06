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
    this->uiproxy = uiproxy;
    connect(this, SIGNAL(create(int,QString)), uiproxy, SLOT(onCreate(int,QString)));
    connect(uiproxy, SIGNAL(buttonClick(int)), this, SLOT(onButtonClick(int)));
    connect(uiproxy, SIGNAL(valueChange(int)), this, SLOT(onValueChange(int)));
}

void UIFunctions::onButtonClick(int id)
{
    try
    {
        LuaCallbackFunction& cb = uiproxy->onclickCallback[id];
        std::cout << "UIFunctions::onButtonClick(" << id << ") calling callback " << cb.function << std::endl;
        onclickCallback_in in_args;
        in_args.id = id;
        onclickCallback_out out_args;
        onclickCallback(cb.scriptID, cb.function.c_str(), &in_args, &out_args);
    }
    catch(std::exception& ex)
    {
    }
}

void UIFunctions::onValueChange(int id)
{
}

