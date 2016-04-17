#include "Window.h"
#include "Widget.h"

#include "XMLUtils.h"

#include "UIProxy.h"

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

    LayoutWidget::parse(e);
}

QWidget * Window::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QDialog *window = new QDialog(parent, Qt::Tool);
    LayoutWidget::createQtWidget(proxy, uiproxy, window);
    window->setWindowTitle(QString::fromStdString(title));
    window->setWindowFlags(Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
    //window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();
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

