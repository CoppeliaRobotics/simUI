#ifndef RADIOBUTTON_H_INCLUDED
#define RADIOBUTTON_H_INCLUDED

#include <vector>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UIProxy;

#include "Widget.h"
#include "Event.h"

class Radiobutton : public Widget, public EventOnClick
{
protected:
    std::string text;
    bool checked;
    bool checkable;
    bool auto_exclusive;

public:
    Radiobutton();
    virtual ~Radiobutton();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    bool convertValueFromInt(int value);
    int convertValueToInt(bool value);
    void setValue(bool value, bool suppressSignals);
    bool getValue();

    friend class UIFunctions;
};

#endif // RADIOBUTTON_H_INCLUDED

