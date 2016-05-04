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

void Window::parse(tinyxml2::XMLElement *e)
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

    LayoutWidget::parse(e);
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
            oncloseCallback_in in_args;
            in_args.handle = window->proxy->getHandle();
            oncloseCallback_out out_args;
            if(oncloseCallback(window->proxy->getScriptID(), window->onclose.c_str(), &in_args, &out_args))
            {
                if(!out_args.accept)
                {
                    event->ignore();
                    return;
                }
            }
            else
            {
                // callback error
            }
        }
        event->accept();
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

