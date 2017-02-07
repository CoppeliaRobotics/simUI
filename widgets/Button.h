#ifndef BUTTON_H_INCLUDED
#define BUTTON_H_INCLUDED

#include <vector>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UIProxy;

#include "Widget.h"
#include "Event.h"

class Button : public Widget, public EventOnClick
{
protected:
    std::string text;
    bool defaulT;
    bool autoRepeat;
    int autoRepeatDelay;
    int autoRepeatInterval;

public:
    Button();
    virtual ~Button();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class UIFunctions;
};

#endif // BUTTON_H_INCLUDED

