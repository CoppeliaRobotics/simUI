#ifndef TABS_H_INCLUDED
#define TABS_H_INCLUDED

#include "config.h"

#include <vector>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UI;

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
    QWidget * createQtWidget(Proxy *proxy, UI *ui, QWidget *parent);

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
    QWidget * createQtWidget(Proxy *proxy, UI *ui, QWidget *parent);

    void setCurrentTab(int index, bool suppressSignals);
    int getCurrentTab();

    friend class SIM;
};

#endif // TABS_H_INCLUDED

