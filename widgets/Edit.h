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

class Edit : public Widget, public EventOnChangeString
{
protected:
    std::string value;

public:
    Edit();
    virtual ~Edit();

    const char * name();
    
    bool parse(tinyxml2::XMLElement *e, std::vector<std::string>& errors);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class UIFunctions;
};

#endif // EDIT_H_INCLUDED

