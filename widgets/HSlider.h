#ifndef HSLIDER_H_INCLUDED
#define HSLIDER_H_INCLUDED

#include <vector>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UIProxy;

#include "Widget.h"
#include "Event.h"

class HSlider : public Widget, public EventOnChangeInt
{
protected:
    int minimum;
    int maximum;

public:
    HSlider();
    virtual ~HSlider();

    bool parse(tinyxml2::XMLElement *e, std::vector<std::string>& errors);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class UIFunctions;
};

#endif // HSLIDER_H_INCLUDED

