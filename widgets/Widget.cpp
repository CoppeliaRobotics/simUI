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
#include "Image.h"

#include "XMLUtils.h"

#include "UIProxy.h"

int Widget::nextId = -1;
std::map<int, Widget *> Widget::widgets;
std::map<QWidget *, Widget *> Widget::widgetByQWidget;

Widget::Widget(std::string widgetClass_)
    : qwidget(NULL), widgetClass(widgetClass_)
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

Widget * Widget::parseAny(tinyxml2::XMLElement *e)
{
    std::string tag(e->Value());
    if(tag == "button") return parse1<Button>(e);
    if(tag == "edit") return parse1<Edit>(e);
    if(tag == "hslider") return parse1<HSlider>(e);
    if(tag == "vslider") return parse1<VSlider>(e);
    if(tag == "label") return parse1<Label>(e);
    if(tag == "checkbox") return parse1<Checkbox>(e);
    if(tag == "radiobutton") return parse1<Radiobutton>(e);
    if(tag == "spinbox") return parse1<Spinbox>(e);
    if(tag == "combobox") return parse1<Combobox>(e);
    if(tag == "group") return parse1<Group>(e);
    if(tag == "tabs") return parse1<Tabs>(e);
    if(tag == "stretch") return parse1<Stretch>(e);
    if(tag == "image") return parse1<Image>(e);

    std::stringstream ss;
    ss << "invalid element <" << tag << ">";
    throw std::range_error(ss.str());
}

void Widget::parse(tinyxml2::XMLElement *e)
{
    if(e->Attribute("id"))
    {
        id = xmlutils::getAttrInt(e, "id", 0);
        // user defined IDs must be positive (see below)
        if(id <= 0)
        {
            throw std::range_error("id must be a positive integer");
        }
    }
    else
    {
        // automatically assigned IDs are negative, so we can distinguish them
        // in diagnostics
        id = nextId--;
    }

    std::string tag(e->Value());
    if(tag != widgetClass)
    {
        std::stringstream ss;
        ss << "element must be <" << widgetClass << ">";
        throw std::range_error(ss.str());
    }
}

std::string Widget::str()
{
    std::stringstream ss;
    ss << "Widget[" << this << "," << widgetClass << "]";
    return ss.str();
}

