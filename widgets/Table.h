#ifndef TABLE_H_INCLUDED
#define TABLE_H_INCLUDED

#include <vector>
#include <string>

#include <QWidget>
#include <QAbstractItemView>

#include "tinyxml2.h"

class Proxy;
class UIProxy;

#include "Widget.h"
#include "Event.h"

struct TableItem
{
    std::string text;
    bool editable;
};

class Table : public Widget
{
protected:
    bool show_header;
    bool show_line_counter;
    bool show_grid;
    bool editable;
    QAbstractItemView::SelectionBehavior selectionBehavior;
    QAbstractItemView::SelectionMode selectionMode;
    std::vector<std::string> header;
    std::vector<std::vector<TableItem> > rows;
    std::string onCellActivate;
    std::string onSelectionChange;

public:
    Table();
    virtual ~Table();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);
    void setEditable(bool editable);
    void clear();
    void setRowCount(int count);
    void setColumnCount(int count);
    void setItem(int row, int column, std::string text);
    int getRowCount();
    int getColumnCount();
    std::string getItem(int row, int column);
    void setColumnHeaderText(int column, std::string text);
    void setItemEditable(int row, int column, bool editable);

    friend class UIFunctions;
};

#endif // TABLE_H_INCLUDED

