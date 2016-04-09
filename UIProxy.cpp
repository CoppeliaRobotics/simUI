#include "UIProxy.h"
#include "widgets/Widget.h"

#include <QWidget>

using namespace tinyxml2;

UIProxy *UIProxy::instance = NULL;

UIProxy::UIProxy(QObject *parent)
    : QObject(parent)
{
}

UIProxy::~UIProxy()
{
}

UIProxy * UIProxy::getInstance(QObject *parent)
{
    if(!UIProxy::instance)
        UIProxy::instance = new UIProxy(parent);
    return UIProxy::instance;
}

void UIProxy::onCreate(Proxy *proxy, Window *window)
{
    proxy->createQtWidget(this, window);
}

void UIProxy::onButtonClick()
{
    QWidget *qwidget = dynamic_cast<QWidget*>(sender());
    if(qwidget)
    {
        Widget *widget = Widget::byQWidget(qwidget);
        if(widget)
        {
            emit buttonClick(widget->id);
        }
    }
}

void UIProxy::onValueChange(int value)
{
    QWidget *qwidget = dynamic_cast<QWidget*>(sender());
    if(qwidget)
    {
        Widget *widget = Widget::byQWidget(qwidget);
        if(widget)
        {
            emit valueChange(widget->id, value);
        }
    }
}

void UIProxy::onValueChange(QString value)
{
    QWidget *qwidget = dynamic_cast<QWidget*>(sender());
    if(qwidget)
    {
        Widget *widget = Widget::byQWidget(qwidget);
        if(widget)
        {
            emit valueChange(widget->id, value);
        }
    }
}

void UIProxy::onDestroy(Proxy *proxy)
{
    proxy->ui->qwidget->close();
    delete proxy;
}

void UIProxy::onDestroyUi(Window *window)
{
    delete window;
}

