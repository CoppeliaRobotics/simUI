#include "Tree.h"

#include "XMLUtils.h"

#include "UIProxy.h"
#include "debug.h"

#include <iostream>
#include <boost/lexical_cast.hpp>

#include <QHeaderView>
#include <QTreeWidget>

Tree::Tree()
    : Widget("tree")
{
}

Tree::~Tree()
{
}

void Tree::parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(parent, widgets, e);

    int nextAutoId = -1;

    for(tinyxml2::XMLElement *e1 = e->FirstChildElement(); e1; e1 = e1->NextSiblingElement())
    {
        std::string tag1(e1->Value());
        if(tag1 == "header")
        {
            for(tinyxml2::XMLElement *e2 = e1->FirstChildElement(); e2; e2 = e2->NextSiblingElement())
            {
                std::string tag2(e2->Value() ? e2->Value() : "");
                if(tag2 != "item") continue;
                std::string itemName(e2->GetText());
                header.push_back(itemName);
            }
        }
        else if(tag1 == "row")
        {
            TreeItem item;

            if(xmlutils::hasAttr(e1, "id"))
            {
                item.id = xmlutils::getAttrInt(e1, "id", 0);
                if(item.id <= 0)
                    throw std::range_error("'id' attribute of element <row> must be positive");
            }
            else
            {
                item.id = nextAutoId--;
            }

            item.parent_id = xmlutils::getAttrInt(e1, "parent-id", 0);

            for(tinyxml2::XMLElement *e2 = e1->FirstChildElement(); e2; e2 = e2->NextSiblingElement())
            {
                std::string tag2(e2->Value() ? e2->Value() : "");
                if(tag2 != "item") continue;
                item.text.push_back(std::string(e2->GetText()));
            }

            item.expanded = xmlutils::getAttrBool(e1, "expanded", false);

            items[item.id] = item;
        }
        else continue;
    }

    show_header = xmlutils::getAttrBool(e, "show-header", true);

    autosize_header = xmlutils::getAttrBool(e, "autosize-header", false);

    sortable = xmlutils::getAttrBool(e, "sortable", false);

    onSelectionChange = xmlutils::getAttrStr(e, "on-selection-change", "");
}

QTreeWidgetItem* Tree::getWidgetItemById(int id)
{
    if(id == 0)
    {
        QTreeWidget *treewidget = static_cast<QTreeWidget*>(getQWidget());
        return treewidget->invisibleRootItem();
    }
    else
    {
        std::map<int, QTreeWidgetItem*>::const_iterator it = widgetItemById.find(id);
        if(it == widgetItemById.end()) return 0L;
        return it->second;
    }
}

QTreeWidgetItem* Tree::makeItem(QTreeWidget *treewidget, const TreeItem &item)
{
    QStringList textList;
    for(size_t i = 0; i < item.text.size(); i++)
        textList.append(QString::fromStdString(item.text[i]));
    QTreeWidgetItem *qtwitem = new QTreeWidgetItem((QTreeWidget*)0, textList);
    widgetItemById[item.id] = qtwitem;
    return qtwitem;
}

void Tree::populateItems(QTreeWidget *treewidget, const std::map<int, std::vector<int> > &by_parent, int parent_id, QTreeWidgetItem *parent)
{
    std::map<int, std::vector<int> >::const_iterator iv = by_parent.find(parent_id);
    if(iv == by_parent.end()) return;
    const std::vector<int> &v = iv->second;
    for(std::vector<int>::const_iterator it = v.begin(); it != v.end(); ++it)
    {
        TreeItem &item = items[*it];
        QTreeWidgetItem *qtwitem = makeItem(treewidget, item);
        parent->addChild(qtwitem);
        qtwitem->setExpanded(item.expanded);
        populateItems(treewidget, by_parent, item.id, qtwitem);
    }
}

QWidget * Tree::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QTreeWidget *treewidget = new QTreeWidget(parent);
    treewidget->setEnabled(enabled);
    treewidget->setVisible(visible);
    treewidget->setStyleSheet(QString::fromStdString(style));
    size_t columncount = header.size();
    treewidget->header()->setVisible(show_header);
    QStringList qtHeader;
    for(size_t i = 0; i < header.size(); i++)
        qtHeader << QString::fromStdString(header[i]);
    treewidget->setHeaderLabels(qtHeader);
    if(autosize_header)
        treewidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    if(sortable)
        treewidget->setSortingEnabled(true);
    std::map<int, std::vector<int> > by_parent;
    for(std::map<int, TreeItem>::iterator it = items.begin(); it != items.end(); ++it)
    {
        columncount = std::max(columncount, it->second.text.size());
        by_parent[it->second.parent_id].push_back(it->first);
    }
    populateItems(treewidget, by_parent, 0, treewidget->invisibleRootItem());
    treewidget->setColumnCount(columncount);
    QObject::connect(treewidget, &QTreeWidget::itemSelectionChanged, uiproxy, &UIProxy::onTreeSelectionChange);
    setQWidget(treewidget);
    setProxy(proxy);
    return treewidget;
}

void Tree::clear(bool suppressSignals)
{
    QTreeWidget *treewidget = static_cast<QTreeWidget*>(getQWidget());
    bool oldSignalsState = treewidget->blockSignals(suppressSignals);
    treewidget->clear(); // or clearContents() ?
    widgetItemById.clear();
    treewidget->blockSignals(oldSignalsState);
}

void Tree::setColumnCount(int count, bool suppressSignals)
{
    if(count < 1) return;
    QTreeWidget *treewidget = static_cast<QTreeWidget*>(getQWidget());
    bool oldSignalsState = treewidget->blockSignals(suppressSignals);
    header.resize(count);
    treewidget->setColumnCount(count);
    treewidget->blockSignals(oldSignalsState);
}

void Tree::addItem(int id, int parent_id, std::vector<std::string> text, bool expanded, bool suppressSignals)
{
    if(id <= 0) return;
    QTreeWidgetItem *parent = getWidgetItemById(parent_id);
    if(!parent) return;
    QTreeWidget *treewidget = static_cast<QTreeWidget*>(getQWidget());
    bool oldSignalsState = treewidget->blockSignals(suppressSignals);
    TreeItem item;
    item.id = id;
    item.parent_id = parent_id;
    item.text = text;
    QTreeWidgetItem *qtwitem = makeItem(treewidget, item);
    parent->addChild(qtwitem);
    qtwitem->setExpanded(expanded);
    treewidget->blockSignals(oldSignalsState);
}

void Tree::updateItemText(int id, std::vector<std::string> text)
{
    if(id == 0) return;
    QTreeWidgetItem *item = getWidgetItemById(id);
    if(!item) return;
    for(int col = 0; col < item->columnCount(); col++)
    {
        QString s = "";
        if(col < text.size())
            s = QString::fromStdString(text[col]);
        item->setText(col, s);
    }
}

void Tree::updateItemParent(int id, int parent_id, bool suppressSignals)
{
    if(id == 0) return;
    QTreeWidgetItem *item = getWidgetItemById(id), *parent = getWidgetItemById(parent_id), *old_parent = item->parent();
    if(!item || !parent || !old_parent) return;
    QTreeWidget *treewidget = static_cast<QTreeWidget*>(getQWidget());
    bool oldSignalsState = treewidget->blockSignals(suppressSignals);
    old_parent->removeChild(item);
    parent->addChild(item);
    treewidget->blockSignals(oldSignalsState);
}

int Tree::getColumnCount()
{
    QTreeWidget *treewidget = static_cast<QTreeWidget*>(getQWidget());
    return treewidget->columnCount();
}

void Tree::removeItem(int id, bool suppressSignals)
{
    if(id == 0) return;
    QTreeWidget *treewidget = static_cast<QTreeWidget*>(getQWidget());
    bool oldSignalsState = treewidget->blockSignals(suppressSignals);
    QTreeWidgetItem *item = getWidgetItemById(id), *parent = item->parent();
    parent->removeChild(item);
    widgetItemById.erase(id);
    treewidget->blockSignals(oldSignalsState);
}

void Tree::setColumnHeaderText(int column, std::string text)
{
    QTreeWidget *treewidget = static_cast<QTreeWidget*>(getQWidget());
    if(column < 0 || column >= header.size()) return;
    header[column] = text;
    QStringList qtHeader;
    for(size_t i = 0; i < header.size(); i++)
        qtHeader << QString::fromStdString(header[i]);
    treewidget->setHeaderLabels(qtHeader);
}

std::string Tree::saveState()
{
    QTreeWidget *treewidget = static_cast<QTreeWidget*>(getQWidget());
    QByteArray hState = treewidget->header()->saveState();
    size_t len1 = hState.length();
    std::string h(hState.constData(), hState.length());
    std::string state = boost::lexical_cast<std::string>(len1) + ":" + h;
    return state;
}

bool Tree::restoreState(std::string state)
{
    QTreeWidget *treewidget = static_cast<QTreeWidget*>(getQWidget());
    size_t d1 = state.find(":");
    std::string slen1 = state.substr(0, d1);
    size_t len1 = boost::lexical_cast<size_t>(slen1);
    const char *s = state.c_str() + d1 + 1;
    QByteArray hState(s, len1);
    bool ret = true;
    ret = ret && treewidget->header()->restoreState(hState);
    return ret;
}

void Tree::setColumnWidth(int column, int min_size, int max_size)
{
    QTreeWidget *treewidget = static_cast<QTreeWidget*>(getQWidget());
    treewidget->header()->setMinimumSectionSize(min_size);
    treewidget->header()->setMaximumSectionSize(max_size);
}

void Tree::setSelection(int id, bool suppressSignals)
{
    if(id == 0) return;
    QTreeWidgetItem *item = getWidgetItemById(id);
    if(!item) return;
    QTreeWidget *treewidget = static_cast<QTreeWidget*>(getQWidget());
    bool oldSignalsState = treewidget->blockSignals(suppressSignals);
    treewidget->setCurrentItem(item);
    treewidget->blockSignals(oldSignalsState);
}

void Tree::expandAll(bool suppressSignals)
{
    QTreeWidget *treewidget = static_cast<QTreeWidget*>(getQWidget());
    bool oldSignalsState = treewidget->blockSignals(suppressSignals);
    treewidget->expandAll();
    treewidget->blockSignals(oldSignalsState);
}

void Tree::collapseAll(bool suppressSignals)
{
    QTreeWidget *treewidget = static_cast<QTreeWidget*>(getQWidget());
    bool oldSignalsState = treewidget->blockSignals(suppressSignals);
    treewidget->collapseAll();
    treewidget->blockSignals(oldSignalsState);
}

void Tree::expandToDepth(int depth, bool suppressSignals)
{
    QTreeWidget *treewidget = static_cast<QTreeWidget*>(getQWidget());
    bool oldSignalsState = treewidget->blockSignals(suppressSignals);
    treewidget->expandToDepth(depth);
    treewidget->blockSignals(oldSignalsState);
}

