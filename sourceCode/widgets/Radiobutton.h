#ifndef RADIOBUTTON_H_INCLUDED
#define RADIOBUTTON_H_INCLUDED

#include "config.h"

#include <vector>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UI;

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
    QWidget * createQtWidget(Proxy *proxy, UI *ui, QWidget *parent);

    bool convertValueFromInt(int value);
    int convertValueToInt(bool value);
    void setValue(bool value, bool suppressSignals);
    bool getValue();

    friend class SIM;
};

#endif // RADIOBUTTON_H_INCLUDED

