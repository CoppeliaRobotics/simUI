#include "VSlider.h"

#include "UIProxy.h"

#include <QSlider>

VSlider::VSlider()
    : Widget()
{
}

VSlider::~VSlider()
{
}

bool VSlider::parse(tinyxml2::XMLElement *e, std::vector<std::string>& errors)
{
    if(!Widget::parse(e, errors)) return false;

    std::string tag(e->Value());
    if(tag != "vslider")
    {
        errors.push_back("element must be <vslider>");
        return false;
    }

    if(!e->Attribute("minimum") || e->QueryIntAttribute("minimum", &minimum) != tinyxml2::XML_NO_ERROR)
        minimum = 0;

    if(!e->Attribute("maximum") || e->QueryIntAttribute("maximum", &maximum) != tinyxml2::XML_NO_ERROR)
        maximum = 100;

    if(e->Attribute("onchange")) onchange = e->Attribute("onchange");
    else onchange = "";

    return true;
}

QWidget * VSlider::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QSlider *slider = new QSlider(Qt::Vertical, parent);
    slider->setMinimum(minimum);
    slider->setMaximum(maximum);
    QObject::connect(slider, SIGNAL(valueChanged(int)), uiproxy, SLOT(onValueChange(int)));
    qwidget = slider;
    Widget::widgetByQWidget[qwidget] = this;
    this->proxy = proxy;
    return slider;
}

