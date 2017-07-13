#ifndef COMBOBOX_H_INCLUDED
#define COMBOBOX_H_INCLUDED

#include "config.h"

#include <vector>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UIProxy;

#include "Widget.h"
#include "Event.h"

class Combobox : public Widget, public EventOnChangeInt
{
protected:
    std::vector<std::string> items;

public:
    Combobox();
    virtual ~Combobox();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    void insertItem(int index, std::string text, bool suppressSignals);
    void removeItem(int index, bool suppressSignals);
    void setItems(std::vector<std::string> items, int index, bool suppressSignals);
    std::vector<std::string> getItems();
    void setSelectedIndex(int index, bool suppressSignals);
    int getSelectedIndex();
    int count();
    std::string itemText(int index);

    friend class UIFunctions;
};

#endif // COMBOBOX_H_INCLUDED

