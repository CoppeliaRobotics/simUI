#ifndef EDIT_H_INCLUDED
#define EDIT_H_INCLUDED

#include <vector>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UIProxy;

#include "Widget.h"
#include "Event.h"

class Edit : public Widget, public EventOnChangeString, public EventOnEditingFinished
{
protected:
    std::string value;

public:
    Edit();
    virtual ~Edit();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class UIFunctions;
};

#endif // EDIT_H_INCLUDED

