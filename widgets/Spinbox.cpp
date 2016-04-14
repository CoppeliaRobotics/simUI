#include "Spinbox.h"

#include "UIProxy.h"

#include <QSpinBox>

Spinbox::Spinbox()
    : Widget()
{
}

Spinbox::~Spinbox()
{
}

const char * Spinbox::name()
{
    return "spinbox";
}

bool Spinbox::parse(tinyxml2::XMLElement *e, std::vector<std::string>& errors)
{
    if(!Widget::parse(e, errors)) return false;

    std::string tag(e->Value());
    if(tag != "spinbox")
    {
        errors.push_back("element must be <spinbox>");
        return false;
    }

    if(!e->Attribute("minimum") || e->QueryIntAttribute("minimum", &minimum) != tinyxml2::XML_NO_ERROR)
        minimum = 0;

    if(!e->Attribute("maximum") || e->QueryIntAttribute("maximum", &maximum) != tinyxml2::XML_NO_ERROR)
        maximum = 100;

    if(e->Attribute("prefix")) prefix = e->Attribute("prefix");
    else prefix = "";

    if(e->Attribute("suffix")) suffix = e->Attribute("suffix");
    else suffix = "";

    if(!e->Attribute("step") || e->QueryIntAttribute("step", &step) != tinyxml2::XML_NO_ERROR)
        step = 1;

    if(e->Attribute("onchange")) onchange = e->Attribute("onchange");
    else onchange = "";

    return true;
}

QWidget * Spinbox::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QSpinBox *slider = new QSpinBox(parent);
    slider->setMinimum(minimum);
    slider->setMaximum(maximum);
    slider->setPrefix(QString::fromStdString(prefix));
    slider->setSuffix(QString::fromStdString(suffix));
    slider->setSingleStep(step);
    QObject::connect(slider, SIGNAL(valueChanged(int)), uiproxy, SLOT(onValueChange(int)));
    qwidget = slider;
    Widget::widgetByQWidget[qwidget] = this;
    this->proxy = proxy;
    return slider;
}

