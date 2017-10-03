#ifndef TABLE_H_INCLUDED
#define TABLE_H_INCLUDED

#include "config.h"

#include <vector>
#include <string>

#include <QWidget>
#include <QAbstractItemView>
#include <QTableWidget>

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

class Table : public Widget, public EventOnKeyPress
{
protected:
    bool show_horizontal_header;
    bool show_vertical_header;
    bool autosize_horizontal_header;
    bool autosize_vertical_header;
    bool show_grid;
    bool editable;
    bool sortable;
    QAbstractItemView::SelectionBehavior selectionBehavior;
    QAbstractItemView::SelectionMode selectionMode;
    std::vector<std::string> horizontalHeader;
    std::vector<std::string> verticalHeader;
    std::vector<std::vector<TableItem> > rows;
    std::string onCellActivate;
    std::string onSelectionChange;

public:
    Table();
    virtual ~Table();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);
    void setEditable(bool editable);
    void clear(bool suppressSignals);
    void setRowCount(int count, bool suppressSignals);
    void setColumnCount(int count, bool suppressSignals);
    void setItem(int row, int column, std::string text, bool suppressSignals);
    void setItemImage(int row, int column, std::string data, int width, int height, bool suppressSignals);
    int getRowCount();
    int getColumnCount();
    std::string getItem(int row, int column);
    void setRowHeaderText(int row, std::string text);
    void setColumnHeaderText(int column, std::string text);
    void setItemEditable(int row, int column, bool editable);
    std::string saveState();
    bool restoreState(std::string state);
    void setRowHeight(int row, int min_size, int max_size);
    void setColumnWidth(int column, int min_size, int max_size);
    void setSelection(int row, int column, bool suppressSignals);

    friend class UIFunctions;
};

class TableWidget : public QTableWidget
{
private:
    Table *table;

public:
    TableWidget(Table *table_, QWidget *parent);
    void keyPressEvent(QKeyEvent *event);
};

#endif // TABLE_H_INCLUDED

