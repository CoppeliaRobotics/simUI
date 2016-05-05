#include "UIProxy.h"
#include "widgets/Widget.h"

#include <QWidget>
#include <QPixmap>
#include <QImage>

using namespace tinyxml2;

UIProxy *UIProxy::instance = NULL;

UIProxy::UIProxy(QObject *parent)
    : QObject(parent)
{
}

UIProxy::~UIProxy()
{
    UIProxy::instance = NULL;
}

UIProxy * UIProxy::getInstance(QObject *parent)
{
    if(!UIProxy::instance)
        UIProxy::instance = new UIProxy(parent);
    return UIProxy::instance;
}

void UIProxy::destroyInstance()
{
    if(UIProxy::instance)
        delete UIProxy::instance;
}

void UIProxy::onCreate(Proxy *proxy)
{
    proxy->createQtWidget(this);
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
    proxy->ui->onclose = ""; // XXX: crash when destroying Window if there is an onclose handler
    proxy->ui->qwidget->close();
    delete proxy;
}

void UIProxy::onDestroyUi(Window *window)
{
    window->onclose = ""; // XXX: crash when destroying Window if there is an onclose handler
    delete window;
}

void UIProxy::onShowWindow(Window *window)
{
    if(window->qwidget_geometry_saved)
    {
        window->qwidget->move(window->qwidget_pos);
        window->qwidget->resize(window->qwidget_size);
    }
    window->qwidget->show();
}

void UIProxy::onHideWindow(Window *window)
{
    window->qwidget_pos = window->qwidget->pos();
    window->qwidget_size = window->qwidget->size();
    window->qwidget_geometry_saved = true;
    window->qwidget->hide();
}

void UIProxy::onSetImage(Image *image, const char *data, int w, int h)
{
    QPixmap pixmap = QPixmap::fromImage(QImage((unsigned char *)data, w, h, QImage::Format_RGB888));
    image->qwidget->setPixmap(pixmap);
    image->qwidget->resize(pixmap->size());
}

