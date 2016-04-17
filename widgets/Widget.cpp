#include "Widget.h"
#include "Button.h"
#include "Edit.h"
#include "HSlider.h"
#include "VSlider.h"
#include "Label.h"
#include "Checkbox.h"
#include "Radiobutton.h"
#include "Spinbox.h"
#include "Combobox.h"
#include "Group.h"
#include "Tabs.h"
#include "LayoutWidget.h"

#include "XMLUtils.h"

#include "UIProxy.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

int Widget::nextId = 1000000;
std::map<int, Widget *> Widget::widgets;
std::map<QWidget *, Widget *> Widget::widgetByQWidget;

Widget::Widget()
    : qwidget(NULL)
{
    // don't do this here because id is set by user:
    // Widget::widgets[id] = this;
}

Widget::~Widget()
{
#ifdef DEBUG
    std::cerr << str() << "::~Widget()" << std::endl;
#endif

    // this should be destroyed from the UI thread

    if(qwidget)
    {
#ifdef DEBUG
        std::cerr << str() << "::~Widget() - delete 'qwidget' member (deleteLater())" << std::endl;
#endif

        //qwidget->deleteLater();

        Widget::widgetByQWidget.erase(qwidget);
    }

    Widget::widgets.erase(id);
}

void Widget::setQWidget(QWidget *qwidget_)
{
    if(qwidget)
        throw std::runtime_error("qwidget has been already set");

    qwidget = qwidget_;
    Widget::widgetByQWidget[qwidget] = this;
}

void Widget::setProxy(Proxy *proxy_)
{
    proxy = proxy_;
}

Widget * Widget::byId(int id)
{
    std::map<int, Widget*>::const_iterator it = Widget::widgets.find(id);
    Widget *ret = it == Widget::widgets.end() ? NULL : it->second;

#ifdef DEBUG
    std::cerr << "Widget::byId(" << id << ") -> " << ret->str() << std::endl;
#endif

    return ret;
}

Widget * Widget::byQWidget(QWidget *w)
{
    std::map<QWidget*, Widget*>::const_iterator it = Widget::widgetByQWidget.find(w);
    Widget *ret = it == Widget::widgetByQWidget.end() ? NULL : it->second;

#ifdef DEBUG
    std::cerr << "Widget::byQWidget(" << w << ") -> " << ret->str() << std::endl;
#endif

    return ret;
}

#ifdef DEBUG
void Widget::dumpTables()
{
    std::cerr << "Widget::dumpTables() - begin dump of widgets:" << std::endl;
    for(std::map<int, Widget *>::const_iterator it = Widget::widgets.begin(); it != Widget::widgets.end(); ++it)
    {
        std::cerr << "  " << it->first << ": " << it->second->str() << std::endl;
    }
    std::cerr << "Widget::dumpTables() - end dump of widgets:" << std::endl;

    std::cerr << "Widget::dumpTables() - begin dump of widgetByQWidget:" << std::endl;
    for(std::map<QWidget *, Widget *>::const_iterator it = Widget::widgetByQWidget.begin(); it != Widget::widgetByQWidget.end(); ++it)
    {
        std::cerr << "  " << it->first << ": " << it->second->str() << std::endl;
    }

    std::cerr << "Widget::dumpTables() - end dump of widgetByQWidget:" << std::endl;
}
#endif

template<typename T>
Widget * Widget::tryParse(tinyxml2::XMLElement *e, std::vector<std::string>& errors)
{
    T *obj = new T;
    if(obj->parse(e, errors))
    {
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
            errors.push_back(ss.str());
            delete obj;
            return NULL;
        }
    }
    else
    {
        delete obj;
        return NULL;
    }
}

Widget * Widget::parseAny(tinyxml2::XMLElement *e, std::vector<std::string>& errors)
{
    std::string tag(e->Value());
    Widget *w = NULL;
    if(tag == "button" && (w = tryParse<Button>(e, errors))) return w;
    if(tag == "edit" && (w = tryParse<Edit>(e, errors))) return w;
    if(tag == "hslider" && (w = tryParse<HSlider>(e, errors))) return w;
    if(tag == "vslider" && (w = tryParse<VSlider>(e, errors))) return w;
    if(tag == "label" && (w = tryParse<Label>(e, errors))) return w;
    if(tag == "checkbox" && (w = tryParse<Checkbox>(e, errors))) return w;
    if(tag == "radiobutton" && (w = tryParse<Radiobutton>(e, errors))) return w;
    if(tag == "spinbox" && (w = tryParse<Spinbox>(e, errors))) return w;
    if(tag == "combobox" && (w = tryParse<Combobox>(e, errors))) return w;
    if(tag == "group" && (w = tryParse<Group>(e, errors))) return w;
    if(tag == "tabs" && (w = tryParse<Tabs>(e, errors))) return w;
    if(tag == "stretch" && (w = tryParse<Stretch>(e, errors))) return w;

    std::stringstream ss;
    ss << "could not parse <" << tag << ">";
    errors.push_back(ss.str());
    return w;
}

bool Widget::parse(tinyxml2::XMLElement *e, std::vector<std::string>& errors)
{
    if(e->Attribute("id"))
        id = xmlutils::getAttrInt(e, "id", 0);
    else
        id = nextId++;

    std::string tag(e->Value());
    if(tag != name())
    {
        std::stringstream ss;
        ss << "element must be <" << name() << ">";
        errors.push_back(ss.str());
        return false;
    }

    return true;
}

const char * Widget::name()
{
    return "???";
}

std::string Widget::str()
{
    std::stringstream ss;
    ss << "Widget[" << this << "," << name() << "]";
    return ss.str();
}

