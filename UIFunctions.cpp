#include "UIFunctions.h"

#include <QThread>

#include <iostream>

UIFunctions::UIFunctions(QObject *parent)
    : QObject(parent)
{
}

UIFunctions::~UIFunctions()
{
}

void UIFunctions::connectToProxy(UIProxy *uiproxy)
{
    connect(this, SIGNAL(createWindow(QString)), uiproxy, SLOT(onCreateWindow(QString)));
}

