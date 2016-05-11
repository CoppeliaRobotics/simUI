#ifndef GROUP_H_INCLUDED
#define GROUP_H_INCLUDED

#include <vector>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UIProxy;

#include "Widget.h"
#include "LayoutWidget.h"

class Group : public Widget, public LayoutWidget
{
protected:
    bool flat;

public:
    Group();
    virtual ~Group();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class UIFunctions;
};

#endif // GROUP_H_INCLUDED

