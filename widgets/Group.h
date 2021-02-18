#ifndef GROUP_H_INCLUDED
#define GROUP_H_INCLUDED

#include "config.h"

#include <vector>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UI;

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
    QWidget * createQtWidget(Proxy *proxy, UI *ui, QWidget *parent);

    friend class SIM;
};

#endif // GROUP_H_INCLUDED

