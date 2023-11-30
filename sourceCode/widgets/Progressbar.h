#ifndef PROGRESSBAR_H_INCLUDED
#define PROGRESSBAR_H_INCLUDED

#include "config.h"

#include <vector>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UI;

#include "Widget.h"

class Progressbar : public Widget
{
protected:
    int minimum;
    int maximum;
    Qt::Orientation orientation;
    bool inverted;
    int value;
    bool text_visible;

public:
    Progressbar();
    virtual ~Progressbar();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UI *ui, QWidget *parent);
    void setValue(int value);

    friend class SIM;
};

#endif // PROGRESSBAR_H_INCLUDED

