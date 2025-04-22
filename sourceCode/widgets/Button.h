#ifndef BUTTON_H_INCLUDED
#define BUTTON_H_INCLUDED

#include "config.h"

#include <vector>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UI;

#include "Widget.h"
#include "Event.h"

class Button : public Widget, public EventOnClick
{
protected:
    std::string text;
    bool defaulT;
    bool auto_repeat;
    int auto_repeat_delay;
    int auto_repeat_interval;
    bool checkable;
    bool checked;
    bool auto_exclusive;
    std::string icon;

public:
    Button();
    virtual ~Button();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UI *ui, QWidget *parent);

    void setText(std::string text);
    void setPressed(bool pressed);

    friend class SIM;
};

#endif // BUTTON_H_INCLUDED

