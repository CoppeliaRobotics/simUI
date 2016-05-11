#ifndef LABEL_H_INCLUDED
#define LABEL_H_INCLUDED

#include <vector>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UIProxy;

#include "Widget.h"

class Label : public Widget
{
protected:
    std::string text;
    bool wordWrap;

public:
    Label();
    virtual ~Label();
    
    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class UIFunctions;
};

#endif // LABEL_H_INCLUDED

