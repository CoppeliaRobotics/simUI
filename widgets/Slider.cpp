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

    if(!e->Attribute("minimum") || e->QueryIntAttribute("minimum", &minimum) != tinyxml2::XML_NO_ERROR)
        minimum = 0;

    if(!e->Attribute("maximum") || e->QueryIntAttribute("maximum", &maximum) != tinyxml2::XML_NO_ERROR)
        maximum = 100;

    if(!e->Attribute("tick-interval") || e->QueryIntAttribute("tick-interval", &tickInterval) != tinyxml2::XML_NO_ERROR)
        tickInterval = 0;

    if(e->Attribute("tick-position"))
    {
        if(strcmp(e->Attribute("tick-position"), "none") == 0)
            tickPosition = QSlider::NoTicks;
        else if(strcmp(e->Attribute("tick-position"), "both-sides") == 0)
            tickPosition = QSlider::TicksBothSides;
        else if(strcmp(e->Attribute("tick-position"), "above") == 0)
            tickPosition = QSlider::TicksAbove;
        else if(strcmp(e->Attribute("tick-position"), "below") == 0)
            tickPosition = QSlider::TicksBelow;
        else if(strcmp(e->Attribute("tick-position"), "left") == 0)
            tickPosition = QSlider::TicksLeft;
        else if(strcmp(e->Attribute("tick-position"), "right") == 0)
            tickPosition = QSlider::TicksRight;
        else
        {
            errors.push_back("invalid value for attribute tick-position");
            return false;
        }
    }
    else
        tickPosition = QSlider::NoTicks;

    if(e->Attribute("inverted") && strcmp(e->Attribute("inverted"), "true") == 0)
        inverted = true;
    else
        inverted = false;

    if(e->Attribute("onchange"))
        onchange = e->Attribute("onchange");
    else
        onchange = "";

    return true;
}

QWidget * Slider::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QSlider *slider = new QSlider(getOrientation(), parent);
    slider->setMinimum(minimum);
    slider->setMaximum(maximum);
    slider->setTickPosition(tickPosition);
    slider->setTickInterval(tickInterval);
    slider->setInvertedAppearance(inverted);
    QObject::connect(slider, SIGNAL(valueChanged(int)), uiproxy, SLOT(onValueChange(int)));
    setQWidget(slider);
    setProxy(proxy);
    return slider;
}


