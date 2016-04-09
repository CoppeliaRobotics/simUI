#include "Window.h"
#include "Widget.h"

#include "UIProxy.h"

#include <iostream>

#include <QDialog>

Window::Window()
    : qwidget(NULL),
      proxy(NULL)
{
}

Window::~Window()
{
#ifdef DEBUG
    std::cerr << "Window::~Window() - contents of Widget::widgets (BEFORE DTOR):" << std::endl;
    for(std::map<int, Widget*>::const_iterator it = Widget::widgets.begin(); it != Widget::widgets.end(); ++it)
        std::cerr << "    " << it->first << ": " << std::hex << ((void*)it->second) << std::dec << std::endl;
    std::cerr << "Window::~Window() - end" << std::endl;

    std::cerr << "Window::~Window() - contents of Widget::widgetByQWidget (BEFORE DTOR):" << std::endl;
    for(std::map<QWidget*, Widget*>::const_iterator it = Widget::widgetByQWidget.begin(); it != Widget::widgetByQWidget.end(); ++it)
        std::cerr << "    " << std::hex << it->first << std::dec << ": " << std::hex << ((void*)it->second) << std::dec << " (id=" << it->second->id << ")" << std::endl;
    std::cerr << "Window::~Window() - end" << std::endl;
#endif

#ifdef DEBUG
    std::cerr << "Window::~Window() - deleting 'children' items" << std::endl;
#endif

    for(std::vector< std::vector<Widget*> >::iterator it = children.begin(); it != children.end(); ++it)
    {
        for(std::vector<Widget*>::iterator it2 = it->begin(); it2 != it->end(); ++it2)
        {
            delete *it2;
        }
    }

    if(qwidget)
    {
#ifdef DEBUG
    std::cerr << "Window::~Window() - deleting member 'qwidget'" << std::endl;
#endif

        delete qwidget;
    }

#ifdef DEBUG
    std::cerr << "Window::~Window() - contents of Widget::widgets (AFTER DTOR):" << std::endl;
    for(std::map<int, Widget*>::const_iterator it = Widget::widgets.begin(); it != Widget::widgets.end(); ++it)
        std::cerr << "    " << it->first << ": " << std::hex << ((void*)it->second) << std::dec << std::endl;
    std::cerr << "Window::~Window() - end" << std::endl;

    std::cerr << "Window::~Window() - contents of Widget::widgetByQWidget (AFTER DTOR):" << std::endl;
    for(std::map<QWidget*, Widget*>::const_iterator it = Widget::widgetByQWidget.begin(); it != Widget::widgetByQWidget.end(); ++it)
        std::cerr << "    " << std::hex << it->first << std::dec << ": " << std::hex << ((void*)it->second) << std::dec << " (id=" << it->second->id << ")" << std::endl;
    std::cerr << "Window::~Window() - end" << std::endl;
#endif
}

bool Window::parse(tinyxml2::XMLElement *e, std::vector<std::string>& errors)
{
    std::string tag(e->Value());
    if(tag != "ui")
    {
        errors.push_back("root element must be <ui>");
        return false;
    }

    if(e->Attribute("title")) title = e->Attribute("title");
    else title = "Custom UI";

    return LayoutWidget::parse(e, errors);
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

