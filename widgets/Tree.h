#ifndef TREE_H_INCLUDED
#define TREE_H_INCLUDED

#include "config.h"

#include <vector>
#include <map>
#include <string>

#include <QWidget>
#include <QAbstractItemView>

#include "tinyxml2.h"

class Proxy;
class UIProxy;

#include "Widget.h"
#include "Event.h"

class QTreeWidget;
class QTreeWidgetItem;

struct TreeItem
{
    int id;
    int parent_id;
    std::vector<std::string> text;
    bool expanded;
};

class Tree : public Widget
{
protected:
    bool show_header;
    bool autosize_header;
    bool sortable;
    std::vector<std::string> header;
    std::map<int, TreeItem> items;
    std::string onSelectionChange;

private:
    void populateItems(QTreeWidget *treewidget, const std::map<int, std::vector<int> > &by_parent, int parent_id, QTreeWidgetItem *parent);
    QTreeWidgetItem* makeItem(QTreeWidget *treewidget, const TreeItem &item);
    std::map<int, QTreeWidgetItem*> widgetItemById;
    QTreeWidgetItem * getWidgetItemById(int id);

public:
    Tree();
    virtual ~Tree();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);
    void clear(bool suppressSignals);
    void setColumnCount(int count, bool suppressSignals);
    void addItem(int id, int parent, std::vector<std::string> text, bool expanded, bool suppressSignals);
    void updateItemText(int id, std::vector<std::string> text);
    void updateItemParent(int id, int parent_id, bool suppressSignals);
    int getColumnCount();
    void removeItem(int id, bool suppressSignals);
    void setColumnHeaderText(int column, std::string text);
    std::string saveState();
    bool restoreState(std::string state);
    void setColumnWidth(int column, int min_size, int max_size);
    void setSelection(int id, bool suppressSignals);
    void expandAll(bool suppressSignals);
    void collapseAll(bool suppressSignals);
    void expandToDepth(int depth, bool suppressSignals);

    friend class UIFunctions;
    friend class UIProxy;
};

#endif // TREE_H_INCLUDED

