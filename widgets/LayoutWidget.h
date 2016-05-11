#ifndef LAYOUTWIDGET_H_INCLUDED
#define LAYOUTWIDGET_H_INCLUDED

#include <vector>
#include <map>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

#include "Widget.h"

class Proxy;
class UIProxy;
class Widget;

enum Layout
{
    VBOX = 1,
    HBOX,
    GRID,
    FORM
};

class Stretch : public Widget
{
protected:
    int factor;

public:
    Stretch();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);    
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class LayoutWidget;
};

class LayoutWidget
{
protected:
    Layout layout;
    std::vector< std::vector<Widget*> > children;

public:
    virtual ~LayoutWidget();

    void parse(Widget *self, Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);    
    void createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class Stretch;
};

#endif // LAYOUTWIDGET_H_INCLUDED

