#ifndef SLIDER_H_INCLUDED
#define SLIDER_H_INCLUDED

#include "config.h"

#include <vector>
#include <string>

#include <QWidget>
#include <QSlider>

#include "tinyxml2.h"

class Proxy;
class UI;

#include "Widget.h"
#include "Event.h"

class Slider : public Widget, public EventOnChangeInt
{
protected:
    int value;
    int minimum;
    int maximum;
    int tickInterval;
    QSlider::TickPosition tickPosition;
    bool inverted;

    virtual Qt::Orientation getOrientation() = 0;

public:
    Slider(std::string widgetClass);
    virtual ~Slider();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UI *ui, QWidget *parent);

    void setValue(int value, bool suppressSignals);
    int getValue();

    friend class SIM;
};

#endif // SLIDER_H_INCLUDED

