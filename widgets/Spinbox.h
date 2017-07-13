#ifndef SPINBOX_H_INCLUDED
#define SPINBOX_H_INCLUDED

#include "config.h"

#include <vector>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UIProxy;

#include "Widget.h"
#include "Event.h"

class Spinbox : public Widget, public EventOnChangeDouble
{
protected:
    double minimum;
    double maximum;
    std::string prefix;
    std::string suffix;
    double step;
    bool float_;

public:
    Spinbox();
    virtual ~Spinbox();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    void setValue(double value, bool suppressSignals);
    double getValue();

    friend class UIFunctions;
};

#endif // SPINBOX_H_INCLUDED

