#include "Slider.h"

#include "XMLUtils.h"

#include "UI.h"

#include <QSlider>

#include <stdexcept>
#include <sstream>

Slider::Slider(std::string widgetClass)
    : Widget(widgetClass)
{
}

Slider::~Slider()
{
}

void Slider::parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(parent, widgets, e);

    value = xmlutils::getAttrInt(e, "value", 0);

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
        std::stringstream ss;
        ss << "invalid value for attribute tick-position: " << tickPositionStr;
        throw std::range_error(ss.str());
    }

    inverted = xmlutils::getAttrBool(e, "inverted", false);

    onchange = xmlutils::getAttrStr(e, "on-change", "");
}

QWidget * Slider::createQtWidget(Proxy *proxy, UI *ui, QWidget *parent)
{
    QSlider *slider = new QSlider(getOrientation(), parent);
    slider->setEnabled(enabled);
    slider->setVisible(visible);
    slider->setStyleSheet(QString::fromStdString(style));
    slider->setMinimum(minimum);
    slider->setMaximum(maximum);
    slider->setValue(value);
    slider->setTickPosition(tickPosition);
    slider->setTickInterval(tickInterval);
    slider->setInvertedAppearance(inverted);
    QObject::connect(slider, &QSlider::valueChanged, ui, &UI::onValueChangeInt);
    setQWidget(slider);
    setProxy(proxy);
    return slider;
}

void Slider::setValue(int value, bool suppressSignals)
{
    QSlider *qslider = static_cast<QSlider*>(getQWidget());
    bool oldSignalsState = qslider->blockSignals(suppressSignals);
    qslider->setValue(value);
    qslider->blockSignals(oldSignalsState);
}

int Slider::getValue()
{
    QSlider *qslider = static_cast<QSlider*>(getQWidget());
    return qslider->value();
}

