#include "Slider.h"

#include "UIProxy.h"

#include <QSlider>

Slider::Slider()
    : Widget()
{
}

Slider::~Slider()
{
}

bool Slider::parse(tinyxml2::XMLElement *e, std::vector<std::string>& errors)
{
    if(!Widget::parse(e, errors)) return false;

    /*
    std::string tag(e->Value());
    if(tag != "hslider")
    {
        errors.push_back("element must be <hslider>");
        return false;
    }
    */

    if(!e->Attribute("minimum") || e->QueryIntAttribute("minimum", &minimum) != tinyxml2::XML_NO_ERROR)
        minimum = 0;

    if(!e->Attribute("maximum") || e->QueryIntAttribute("maximum", &maximum) != tinyxml2::XML_NO_ERROR)
        maximum = 100;

    if(e->Attribute("onchange")) onchange = e->Attribute("onchange");
    else onchange = "";

    return true;
}

QWidget * Slider::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QSlider *slider = new QSlider(getOrientation(), parent);
    slider->setMinimum(minimum);
    slider->setMaximum(maximum);
    QObject::connect(slider, SIGNAL(valueChanged(int)), uiproxy, SLOT(onValueChange(int)));
    qwidget = slider;
    Widget::widgetByQWidget[qwidget] = this;
    this->proxy = proxy;
    return slider;
}


