#ifndef CHECKBOX_H_INCLUDED
#define CHECKBOX_H_INCLUDED

#include <vector>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UIProxy;

#include "Widget.h"
#include "Event.h"

class Checkbox : public Widget, public EventOnChangeInt
{
protected:
    std::string text;
    bool checked;
    bool checkable;
    bool auto_exclusive;

public:
    Checkbox();
    virtual ~Checkbox();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    Qt::CheckState convertValueFromInt(int value);
    int convertValueToInt(Qt::CheckState value);
    void setValue(Qt::CheckState value, bool suppressSignals);
    Qt::CheckState getValue();

    friend class UIFunctions;
};

#endif // CHECKBOX_H_INCLUDED

