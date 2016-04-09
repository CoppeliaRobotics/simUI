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

public:
    Checkbox();
    virtual ~Checkbox();

    bool parse(tinyxml2::XMLElement *e, std::vector<std::string>& errors);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class UIFunctions;
};

#endif // CHECKBOX_H_INCLUDED

