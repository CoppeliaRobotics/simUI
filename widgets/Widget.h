#ifndef WIDGET_H_INCLUDED
#define WIDGET_H_INCLUDED

#include <stdexcept>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <iostream>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UIProxy;

class Widget
{
private:
    QWidget *qwidget;
    Proxy *proxy;
    const std::string widgetClass;

protected:
    static std::map<QWidget *, Widget *> widgetByQWidget;
    static std::set<Widget *> widgets;

    int id;
    std::string style;
    bool enabled;
    bool visible;

    Widget(std::string widgetClass);

    void setQWidget(QWidget *qwidget);
    void setProxy(Proxy *proxy);

public:
    virtual ~Widget();

    virtual void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    virtual QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent) = 0;

    inline int getId() {return id;}
    inline QWidget * getQWidget() {return qwidget;}

    template<typename T>
    static T * parse1(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    static Widget * parseAny(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);

    static Widget * byId(int handle, int id);
    static Widget * byQWidget(QWidget *w);

    static bool exists(Widget *w);

    friend class UIFunctions;
    friend class UIProxy;
    friend class Window;
};

template<typename T>
T * Widget::parse1(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    T *obj = new T;
    try
    {
        obj->parse(parent, widgets, e);

        // object parsed successfully
        // now check if ID is duplicate:
        if(widgets.find(obj->id) != widgets.end())
        {
            std::stringstream ss;
            ss << "id must be unique (within ui scope). duplicate id: " << obj->id;
            throw std::range_error(ss.str());
        }
        else
        {
            widgets[obj->id] = obj;
            return obj;
        }
    }
    catch(std::exception& ex)
    {
        delete obj;
        std::stringstream ss;
        ss << e->Value();
        if(obj->id > 0)
        {
            ss << "[id=" << obj->id << "]";
        }
        ss << ": " << ex.what();
        throw std::range_error(ss.str());
    }
}

#endif // WIDGET_H_INCLUDED

