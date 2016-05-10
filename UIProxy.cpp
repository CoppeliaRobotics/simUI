#include "UIProxy.h"
#include "widgets/Widget.h"

#include <QWidget>
#include <QPixmap>
#include <QImage>
#include <QLabel>

#include "stubs.h"

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
            emit buttonClick(widget);
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
            emit valueChange(widget, value);
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
            emit valueChange(widget, value);
        }
    }
}

void UIProxy::onDestroy(Proxy *proxy)
{
#ifdef DEBUG
    std::cerr << "UIProxy->onDestroy(" << (void*)proxy << ")" << std::endl;
#endif // DEBUG

    proxy->ui->onclose = ""; // XXX: crash when destroying Window if there is an onclose handler
    proxy->ui->qwidget->close();
    delete proxy;
}

void UIProxy::onDestroyUi(Window *window)
{
#ifdef DEBUG
    std::cerr << "UIProxy->onDestroyUi(" << (void*)window << ")" << std::endl;
#endif // DEBUG

    window->onclose = ""; // XXX: crash when destroying Window if there is an onclose handler
    delete window;
}

void UIProxy::onShowWindow(Window *window)
{
#ifdef DEBUG
    std::cerr << "UIProxy->onShowWindow(" << (void*)window << ")" << std::endl;
#endif // DEBUG

    window->show();
}

void UIProxy::onHideWindow(Window *window)
{
#ifdef DEBUG
    std::cerr << "UIProxy->onHideWindow(" << (void*)window << ")" << std::endl;
#endif // DEBUG

    window->hide();
}

void UIProxy::onSetImage(Image *image, const char *data, int w, int h)
{
#ifdef DEBUG
    std::cerr << "UIProxy->onSetImage(" << (void*)image << ", " << (void*)data << ", " << w << ", " << h << ")" << std::endl;
#endif // DEBUG

    QImage::Format format = QImage::Format_RGB888;
    int bpp = 3; // bytes per pixel
    QPixmap pixmap = QPixmap::fromImage(QImage((unsigned char *)data, w, h, bpp * w, format));
    simReleaseBufferE((char *)data); // XXX: simReleaseBuffer should accept a const pointer
    QLabel *label = static_cast<QLabel*>(image->qwidget);
    label->setPixmap(pixmap);
    label->resize(pixmap.size());
}

void UIProxy::onSceneChange(Window *window, int oldSceneID, int newSceneID)
{
#ifdef DEBUG
    std::cerr << "UIProxy->onSceneChange(" << (void*)window << ", " << oldSceneID << ", " << newSceneID << ")" << std::endl;
#endif // DEBUG

    window->onSceneChange(oldSceneID, newSceneID);
}

