#ifndef TABS_H_INCLUDED
#define TABS_H_INCLUDED

#include <vector>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UIProxy;

#include "Widget.h"
#include "LayoutWidget.h"

class Tab : public Widget, public LayoutWidget
{
protected:
    std::string title;

public:
    Tab();
    virtual ~Tab();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class Tabs;
};

class Tabs : public Widget
{
protected:
    std::vector<Tab*> tabs;

public:
    Tabs();
    virtual ~Tabs();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class UIFunctions;
};

#endif // TABS_H_INCLUDED

