#ifndef LABEL_H_INCLUDED
#define LABEL_H_INCLUDED

#include "config.h"

#include <vector>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UI;

#include "Widget.h"
#include "Event.h"

class Label : public Widget, public EventOnLinkActivated
{
protected:
    std::string text;
    bool wordWrap;

public:
    Label();
    virtual ~Label();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UI *ui, QWidget *parent);

    void setText(std::string text, bool suppressSignals);
    std::string getText();

    friend class SIM;
};

#endif // LABEL_H_INCLUDED

