#ifndef CHECKBOX_H_INCLUDED
#define CHECKBOX_H_INCLUDED

#include "config.h"

#include <vector>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UI;

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
    QWidget * createQtWidget(Proxy *proxy, UI *ui, QWidget *parent);

    Qt::CheckState convertValueFromInt(int value);
    int convertValueToInt(Qt::CheckState value);
    void setValue(Qt::CheckState value, bool suppressSignals);
    Qt::CheckState getValue();

    friend class SIM;
};

#endif // CHECKBOX_H_INCLUDED

