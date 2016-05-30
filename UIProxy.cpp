#include "UIProxy.h"
#include "widgets/Widget.h"

#include <QThread>
#include <QWidget>
#include <QPixmap>
#include <QImage>
#include <QLabel>

#include "stubs.h"

using namespace tinyxml2;

// UIProxy is a singleton

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
    {
        UIProxy::instance = new UIProxy(parent);
#ifdef DEBUG
        std::cerr << "UIProxy constructed in thread " << QThread::currentThreadId() << std::endl;
#endif // DEBUG
    }
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

// The following slots are directly connected to Qt Widgets' signals.
// Here we look up the sender to find the corresponding Widget object,
// and emit a signal with the Widget object pointer.
//
// That signal will be connected to a slot in UIFunctions, such
// that the callback is called from the SIM thread.

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

void UIProxy::onEditingFinished()
{
    QWidget *qwidget = dynamic_cast<QWidget*>(sender());

#ifdef DEBUG
    std::cerr << "UIProxy::onEditingFinished()" << std::endl;
#endif // DEBUG

    if(qwidget)
    {
        Widget *widget = Widget::byQWidget(qwidget);

#ifdef DEBUG
        std::cerr << "    qwidget = " << qwidget << std::endl;
#endif // DEBUG

        if(widget)
        {
#ifdef DEBUG
            std::cerr << "    widget->id = " << widget->id << std::endl;
            std::cerr << "    widget->widgetClass = " << widget->widgetClass << std::endl;
#endif // DEBUG

            emit editingFinished(widget);
        }
        else
        {
#ifdef DEBUG
            std::cerr << "    not mapped to a Widget object" << std::endl;
#endif // DEBUG
        }
    }
    else
    {
#ifdef DEBUG
        std::cerr << "    sender() is not a QWidget" << std::endl;
#endif // DEBUG
    }
}

// The following slots are wrappers for functions called from SIM thread
// which should instead execute in the UI thread.

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

void UIProxy::onSetEnabled(Widget *widget, bool enabled)
{
#ifdef DEBUG
    std::cerr << "UIProxy->onSetEnabled(" << (void*)widget << ", " << enabled << ")" << std::endl;
#endif // DEBUG

    widget->getQWidget()->setEnabled(enabled);
}

