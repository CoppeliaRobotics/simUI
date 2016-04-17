#include "Slider.h"

#include "XMLUtils.h"

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

    minimum = xmlutils::getAttrInt(e, "minimum", 0);

    maximum = xmlutils::getAttrInt(e, "maximum", 100);

    tickInterval = xmlutils::getAttrInt(e, "tick-interval", 0);

    std::string tickPositionStr = xmlutils::getAttrStr(e, "tick-position", "none");
    if(tickPositionStr ==  "none") tickPosition = QSlider::NoTicks;
    else if(tickPositionStr == "both-sides") tickPosition = QSlider::TicksBothSides;
    else if(tickPositionStr == "above") tickPosition = QSlider::TicksAbove;
    else if(tickPositionStr == "below") tickPosition = QSlider::TicksBelow;
    else if(tickPositionStr == "left") tickPosition = QSlider::TicksLeft;
    else if(tickPositionStr == "right") tickPosition = QSlider::TicksRight;
    else
    {
        errors.push_back("invalid value for attribute tick-position");
        return false;
    }

    inverted = xmlutils::getAttrBool(e, "inverted", false);

    onchange = xmlutils::getAttrStr(e, "onchange", "");

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


