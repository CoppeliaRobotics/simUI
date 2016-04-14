#include "Combobox.h"

#include "UIProxy.h"

#include <iostream>

#include <QComboBox>

Combobox::Combobox()
    : Widget()
{
}

Combobox::~Combobox()
{
}

const char * Combobox::name()
{
    return "combobox";
}

bool Combobox::parse(tinyxml2::XMLElement *e, std::vector<std::string>& errors)
{
    if(!Widget::parse(e, errors)) return false;

    std::string tag(e->Value());
    if(tag != "combobox")
    {
        errors.push_back("element must be <combobox>");
        return false;
    }

    for(tinyxml2::XMLElement *e1 = e->FirstChildElement(); e1; e1 = e1->NextSiblingElement())
    {
        std::string tag1(e1->Value());
        if(tag1 != "item") continue;
        std::string itemName(e1->GetText());
        items.push_back(itemName);
    }    

    if(e->Attribute("onchange")) onchange = e->Attribute("onchange");
    else onchange = "";

    return true;
}

QWidget * Combobox::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QComboBox *combobox = new QComboBox(parent);
    for(std::vector<std::string>::const_iterator it = items.begin(); it != items.end(); ++it)
    {
        combobox->addItem(QString::fromStdString(*it));
    }
    QObject::connect(combobox, SIGNAL(currentIndexChanged(int)), uiproxy, SLOT(onValueChange(int)));
    qwidget = combobox;
    Widget::widgetByQWidget[qwidget] = this;
    this->proxy = proxy;
    return combobox;
}

