#ifndef COMBOBOX_H_INCLUDED
#define COMBOBOX_H_INCLUDED

#include <vector>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UIProxy;

#include "Widget.h"
#include "Event.h"

class Combobox : public Widget, public EventOnChangeInt
{
protected:
    std::vector<std::string> items;

public:
    Combobox();
    virtual ~Combobox();
    
    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class UIFunctions;
};

#endif // COMBOBOX_H_INCLUDED

