#ifndef TABLE_H_INCLUDED
#define TABLE_H_INCLUDED

#include <vector>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UIProxy;

#include "Widget.h"
#include "Event.h"

class Table : public Widget, public EventOnChangeString
{
protected:
    bool show_header;
    bool show_line_counter;
    bool show_grid;
    bool editable;
    int select_mode;
    std::vector<std::string> header;
    std::vector<std::vector<std::string> > rows;
    std::string onselectionchange;

public:
    Table();
    virtual ~Table();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);
    void setEditable(bool editable);

    friend class UIFunctions;
};

#endif // TABLE_H_INCLUDED

