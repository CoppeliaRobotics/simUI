#include "Window.h"
#include "Widget.h"

#include "XMLUtils.h"

#include "UIProxy.h"

#include "stubs.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

#include <QDialog>

Window::Window()
    : qwidget(NULL),
      qwidget_geometry_saved(false),
      visibility_state(true),
      proxy(NULL)
{
}

Window::~Window()
{
    if(qwidget)
    {
#ifdef DEBUG
        std::cerr << "Window::~Window() - deleting member 'qwidget'" << std::endl;
#endif

        delete qwidget;
    }

#ifdef DEBUG
    std::cerr << "Window::~Window() - AFTER DTOR:" << std::endl;
    Widget::dumpTables();
#endif
}

void Window::parse(std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    std::string tag(e->Value());
    if(tag != "ui")
    {
        throw std::range_error("root element must be <ui>");
    }

    title = xmlutils::getAttrStr(e, "title", "Custom UI");

    resizable = xmlutils::getAttrBool(e, "resizable", false);

    closeable = xmlutils::getAttrBool(e, "closeable", false);

    onclose = xmlutils::getAttrStr(e, "onclose", "");

    LayoutWidget::parse(widgets, e);
}

#include <QCloseEvent>

class QDialog2 : public QDialog
{
protected:
    Window *window;

public:
    QDialog2(Window *window_, QWidget *parent = 0)
        : QDialog(parent),
          window(window_)
    {
    }

    virtual void closeEvent(QCloseEvent * event)
    {
        if(window->onclose != "")
        {
            event->ignore();
            UIProxy::getInstance()->windowClose(window);
        }
        else
        {
            event->accept();
        }
    }
};

QWidget * Window::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QDialog *window = new QDialog2(this, parent);
    LayoutWidget::createQtWidget(proxy, uiproxy, window);
    window->setWindowTitle(QString::fromStdString(title));
    Qt::WindowFlags flags = Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint;
#ifdef MAC_VREP
    flags |= Qt::Tool;
#else
#ifdef LIN_VREP
    flags |= Qt::Window;
#else
    flags |= Qt::Dialog;
#endif
#endif
    if(resizable) flags |= Qt::WindowMaximizeButtonHint;
    else flags |= Qt::MSWindowsFixedSizeDialogHint;
    if(closeable) flags |= Qt::WindowCloseButtonHint;
    window->setWindowFlags(flags);
    //window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();
#ifdef LIN_VREP
    if(!resizable) window->setFixedSize(window->size());
#endif
    qwidget = window;
    this->proxy = proxy;
    return window;
}

std::string Window::str()
{
    std::stringstream ss;
    ss << "Window[" << this << "]";
    return ss.str();
}

void Window::hide()
{
    qwidget_pos = qwidget->pos();
    qwidget_size = qwidget->size();
    qwidget_geometry_saved = true;
    visibility_state = false;
    qwidget->hide();
}

void Window::show()
{
    if(qwidget_geometry_saved)
    {
        qwidget->move(qwidget_pos);
        qwidget->resize(qwidget_size);
    }
    visibility_state = true;
    qwidget->show();
}

void Window::onSceneChange(int oldSceneID, int newSceneID)
{
    int mySceneID = proxy->getSceneID();

#ifdef DEBUG
        std::cerr << "Window->onSceneChange(" << oldSceneID << ", " << newSceneID << ") [mySceneID=" << mySceneID << "]" << std::endl;
#endif // DEBUG

    if(oldSceneID == mySceneID && newSceneID != mySceneID)
    {
#ifdef DEBUG
        std::cerr << "Window->onSceneChange: hidden" << std::endl;
#endif // DEBUG

        bool saved_visibility_state = visibility_state;
        hide();
        visibility_state = saved_visibility_state;
    }

    if(oldSceneID != mySceneID && newSceneID == mySceneID)
    {
#ifdef DEBUG
        std::cerr << "Window->onSceneChange: shown" << std::endl;
#endif // DEBUG

        if(visibility_state)
            show();
    }
}

