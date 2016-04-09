#ifndef VSLIDER_H_INCLUDED
#define VSLIDER_H_INCLUDED

#include <vector>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UIProxy;

#include "Widget.h"
#include "Event.h"

class VSlider : public Widget, public EventOnChangeInt
{
protected:
    int minimum;
    int maximum;

public:
    VSlider();
    virtual ~VSlider();

    bool parse(tinyxml2::XMLElement *e, std::vector<std::string>& errors);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class UIFunctions;
};

#endif // VSLIDER_H_INCLUDED

