#ifndef WIDGET_H_INCLUDED
#define WIDGET_H_INCLUDED

#include <stdexcept>
#include <vector>
#include <map>
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
    static int nextId;
    static std::map<int, Widget *> widgets;
    static std::map<QWidget *, Widget *> widgetByQWidget;

    int id;

    Widget(std::string widgetClass);

    void setQWidget(QWidget *qwidget);
    void setProxy(Proxy *proxy);

public:
    virtual ~Widget();

    virtual void parse(tinyxml2::XMLElement *e);
    virtual QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent) = 0;

    inline QWidget * getQWidget() {return qwidget;}

    template<typename T>
    static T * parse1(tinyxml2::XMLElement *e);
    static Widget * parseAny(tinyxml2::XMLElement *e);

    static Widget * byId(int id);
    static Widget * byQWidget(QWidget *w);

    std::string str();

#ifdef DEBUG
    static void dumpTables();
#endif

    friend class UIFunctions;
    friend class UIProxy;
    friend class Window;
};

template<typename T>
T * Widget::parse1(tinyxml2::XMLElement *e)
{
    T *obj = new T;
    try
    {
        obj->parse(e);

        // object parsed successfully
        // now check if ID is duplicate:
        std::map<int, Widget *>::const_iterator it = Widget::widgets.find(obj->id);
        if(it == Widget::widgets.end())
        {
            Widget::widgets[obj->id] = obj;
            return obj;
        }
        else
        {
            std::stringstream ss;
            ss << "duplicate widget id: " << obj->id;
#ifdef DEBUG
            std::cerr << ss.str() << std::endl;
            Widget::dumpTables();
#endif
            delete obj;
            throw std::range_error(ss.str());
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

