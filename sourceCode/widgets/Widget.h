#ifndef WIDGET_H_INCLUDED
#define WIDGET_H_INCLUDED

#include "config.h"

#include <stdexcept>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <iostream>

#include <boost/format.hpp>

#include <QWidget>

#include "simPlusPlus/Handles.h"

#include "XMLUtils.h"

class Proxy;
class UI;

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
    struct {
        int x, y, width, height;
        bool isSet;
    } geometry;
    std::string style;
    bool enabled;
    bool visible;

    Widget(std::string widgetClass);

    void setQWidget(QWidget *qwidget);
    void setProxy(Proxy *proxy);

public:
    virtual ~Widget();

    virtual void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    virtual QWidget * createQtWidget(Proxy *proxy, UI *ui, QWidget *parent) = 0;

    inline int getId() {return id;}
    inline QWidget * getQWidget() {return qwidget;}

    template<typename T>
    static T * parse1(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    static Widget * parseAny(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);

    static Widget * byId(const std::string &handle, int id, const sim::Handles<Proxy*> &handles);
    static Widget * byQWidget(QWidget *w);

    static bool exists(Widget *w);

    friend class SIM;
    friend class UI;
    friend class Window;
    friend class LayoutWidget;
};

template<typename T>
T * Widget::parse1(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    T *obj = new T;
    try
    {
        xmlutils::resetKnownAttributes();
        obj->parse(parent, widgets, e);
        xmlutils::reportUnknownAttributes(obj->widgetClass, e);

        // object parsed successfully
        // now check if ID is duplicate:
        if(widgets.find(obj->id) != widgets.end())
        {
            throw std::range_error((boost::format("id must be unique (within ui scope). duplicate id: %d") % obj->id).str());
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
        throw std::range_error((boost::format("%s%s: %s") % e->Value() % (obj->id > 0 ? (boost::format("[id=%d]") % obj->id).str() : std::string("")) % ex.what()).str());
    }
}

#endif // WIDGET_H_INCLUDED

