#include "Widget.h"
#include "Button.h"
#include "Edit.h"
#include "HSlider.h"
#include "VSlider.h"
#include "Label.h"
#include "Checkbox.h"
#include "Radiobutton.h"
#include "Spinbox.h"
#include "Group.h"

#include "UIProxy.h"

#include <iostream>
#include <sstream>

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
    std::cerr << "Widget::~Widget() - this=" << std::hex << ((void*)this) << std::dec << std::endl;
#endif

    // this should be destroyed from the UI thread

    if(qwidget)
    {
#ifdef DEBUG
    std::cerr << "Widget::~Widget() - delete 'qwidget' member (deleteLater())" << std::endl;
#endif

        qwidget->deleteLater();

        Widget::widgetByQWidget.erase(qwidget);
    }

    Widget::widgets.erase(id);
}

Widget * Widget::byId(int id)
{
    std::map<int, Widget*>::const_iterator it = Widget::widgets.find(id);
    Widget *ret = it == Widget::widgets.end() ? NULL : it->second;

#ifdef DEBUG
    std::cerr << "Widget::byId(" << id << ") -> " << std::hex << ret << std::dec << std::endl;
#endif

    return ret;
}

Widget * Widget::byQWidget(QWidget *w)
{
    std::map<QWidget*, Widget*>::const_iterator it = Widget::widgetByQWidget.find(w);
    Widget *ret = it == Widget::widgetByQWidget.end() ? NULL : it->second;

#ifdef DEBUG
    std::cerr << "Widget::byQWidget(" << std::hex << w << std::dec << ") -> " << std::hex << ret << std::dec << std::endl;
#endif

    return ret;
}

template<typename T>
Widget * Widget::tryParse(tinyxml2::XMLElement *e, std::vector<std::string>& errors)
{
    T *obj = new T;
    if(obj->parse(e, errors))
    {
        Widget::widgets[obj->id] = obj;
        return obj;
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
    if(tag == "group" && (w = tryParse<Group>(e, errors))) return w;

    std::stringstream ss;
    ss << "could not parse <" << tag << ">";
    errors.push_back(ss.str());
    return w;
}

bool Widget::parse(tinyxml2::XMLElement *e, std::vector<std::string>& errors)
{
    if(!e->Attribute("id") || e->QueryIntAttribute("id", &id) != tinyxml2::XML_NO_ERROR)
    {
        id = nextId++;
    }

    return true;
}

