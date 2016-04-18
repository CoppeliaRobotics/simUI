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

public:
    Checkbox();
    virtual ~Checkbox();

    const char * name();
    
    void parse(tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class UIFunctions;
};

#endif // CHECKBOX_H_INCLUDED

