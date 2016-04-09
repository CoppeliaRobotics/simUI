#include "Group.h"

#include "UIProxy.h"

#include <QGroupBox>

Group::Group()
    : Widget()
{
}

Group::~Group()
{
    for(std::vector< std::vector<Widget*> >::iterator it = children.begin(); it != children.end(); ++it)
    {
        for(std::vector<Widget*>::iterator it2 = it->begin(); it2 != it->end(); ++it2)
        {
            delete *it2;
        }
    }
}

bool Group::parse(tinyxml2::XMLElement *e, std::vector<std::string>& errors)
{
    if(!Widget::parse(e, errors)) return false;

    std::string tag(e->Value());
    if(tag != "group")
    {
        errors.push_back("element must be <group>");
        return false;
    }

    if(e->Attribute("text")) text = e->Attribute("text");
    else text = "";

    return LayoutWidget::parse(e, errors);
}

QWidget * Group::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QGroupBox *groupBox = new QGroupBox(QString::fromStdString(text), parent);
    LayoutWidget::createQtWidget(proxy, uiproxy, groupBox);
    qwidget = groupBox;
    Widget::widgetByQWidget[qwidget] = this;
    this->proxy = proxy;
    return groupBox;
}

