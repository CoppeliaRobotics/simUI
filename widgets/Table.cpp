#include "Table.h"

#include "XMLUtils.h"

#include "UIProxy.h"
#include "debug.h"

#include <iostream>

#include <QTableWidget>

#if __cplusplus <= 199711L
#include <cstdlib>
namespace std {
    static size_t stol(const std::string &s) {
        return atol(s.c_str());
    }
}
#endif

Table::Table()
    : Widget("table")
{
}

Table::~Table()
{
}

void Table::parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(parent, widgets, e);

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
                horizontalHeader.push_back(itemName);
            }
        }
        else if(tag1 == "row")
        {
            rows.resize(rows.size()+1);
            std::string rowHeader = xmlutils::getAttrStr(e1, "label", boost::lexical_cast<std::string>(rows.size()));
            verticalHeader.push_back(rowHeader);
            for(tinyxml2::XMLElement *e2 = e1->FirstChildElement(); e2; e2 = e2->NextSiblingElement())
            {
                std::string tag2(e2->Value() ? e2->Value() : "");
                if(tag2 != "item") continue;
                TableItem item;
                item.text = std::string(e2->GetText());
                item.editable = xmlutils::getAttrBool(e2, "editable", true);
                rows[rows.size()-1].push_back(item);
            }
        }
        else continue;
    }

    show_horizontal_header = xmlutils::getAttrBool(e, "show-horizontal-header", true);

    show_vertical_header = xmlutils::getAttrBool(e, "show-vertical-header", false);

    autosize_horizontal_header = xmlutils::getAttrBool(e, "autosize-horizontal-header", false);

    autosize_vertical_header = xmlutils::getAttrBool(e, "autosize-vertical-header", true);

    show_grid = xmlutils::getAttrBool(e, "show-grid", true);

    editable = xmlutils::getAttrBool(e, "editable", true);

    sortable = xmlutils::getAttrBool(e, "sortable", false);

    onCellActivate = xmlutils::getAttrStr(e, "on-cell-activate", "");

    onSelectionChange = xmlutils::getAttrStr(e, "on-selection-change", "");

    std::string select_mode_str = xmlutils::getAttrStr(e, "selection-mode", "item");
    selectionMode = QAbstractItemView::SingleSelection;
    if(select_mode_str == "item") selectionBehavior = QAbstractItemView::SelectItems;
    else if(select_mode_str == "row") selectionBehavior = QAbstractItemView::SelectRows;
    else if(select_mode_str == "column") selectionBehavior = QAbstractItemView::SelectColumns;
    else throw std::range_error("selection-mode must be one of: 'item', 'row', 'column'");
}

QWidget * Table::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QTableWidget *tablewidget = new QTableWidget(parent);
    tablewidget->setEnabled(enabled);
    tablewidget->setVisible(visible);
    tablewidget->setStyleSheet(QString::fromStdString(style));
    size_t rowcount = rows.size(), columncount = 0;
    for(size_t i = 0; i < rowcount; i++)
        columncount = std::max(columncount, rows[i].size());
    tablewidget->setRowCount(rowcount);
    tablewidget->setColumnCount(columncount);
    tablewidget->horizontalHeader()->setVisible(show_horizontal_header);
    tablewidget->verticalHeader()->setVisible(show_vertical_header);
    tablewidget->setShowGrid(show_grid);
    QStringList qtHorizontalHeader;
    for(size_t i = 0; i < horizontalHeader.size(); i++)
        qtHorizontalHeader << QString::fromStdString(horizontalHeader[i]);
    tablewidget->setHorizontalHeaderLabels(qtHorizontalHeader);
    QStringList qtVerticalHeader;
    for(size_t i = 0; i < verticalHeader.size(); i++)
        qtVerticalHeader << QString::fromStdString(verticalHeader[i]);
    tablewidget->setVerticalHeaderLabels(qtVerticalHeader);
    if(autosize_horizontal_header)
        tablewidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    if(autosize_vertical_header)
        tablewidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    if(sortable)
        tablewidget->setSortingEnabled(true);
    for(size_t row = 0; row < rowcount; row++)
    {
        for(size_t column = 0; column < rows[row].size(); column++)
        {
            TableItem &item = rows[row][column];
            QTableWidgetItem *qtwitem = new QTableWidgetItem(QString::fromStdString(item.text));
            if(item.editable)
                qtwitem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            else
                qtwitem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            tablewidget->setItem(row, column, qtwitem);
        }
    }
    tablewidget->setSelectionBehavior(selectionBehavior);
    tablewidget->setSelectionMode(selectionMode);
    QObject::connect(tablewidget, &QTableWidget::cellActivated, uiproxy, &UIProxy::onCellActivate);
    QObject::connect(tablewidget, &QTableWidget::cellChanged, uiproxy, &UIProxy::onCellActivate);
    QObject::connect(tablewidget, &QTableWidget::itemSelectionChanged, uiproxy, &UIProxy::onTableSelectionChange);
    setQWidget(tablewidget);
    setEditable(editable);
    setProxy(proxy);
    return tablewidget;
}

void Table::setEditable(bool editable)
{
    QTableWidget *tablewidget = static_cast<QTableWidget*>(getQWidget());
    if(editable)
    {
        // TODO
    }
    else
    {
        tablewidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
}

void Table::clear()
{
    QTableWidget *tablewidget = static_cast<QTableWidget*>(getQWidget());
    tablewidget->clear(); // or clearContents() ?
}

void Table::setRowCount(int count)
{
    QTableWidget *tablewidget = static_cast<QTableWidget*>(getQWidget());
    tablewidget->setRowCount(count);
}

void Table::setColumnCount(int count)
{
    QTableWidget *tablewidget = static_cast<QTableWidget*>(getQWidget());
    tablewidget->setColumnCount(count);
}

void Table::setItem(int row, int column, std::string text)
{
    QTableWidget *tablewidget = static_cast<QTableWidget*>(getQWidget());
    tablewidget->setItem(row, column, new QTableWidgetItem(QString::fromStdString(text)));
}

int Table::getRowCount()
{
    QTableWidget *tablewidget = static_cast<QTableWidget*>(getQWidget());
    return tablewidget->rowCount();
}

int Table::getColumnCount()
{
    QTableWidget *tablewidget = static_cast<QTableWidget*>(getQWidget());
    return tablewidget->columnCount();
}

std::string Table::getItem(int row, int column)
{
    QTableWidget *tablewidget = static_cast<QTableWidget*>(getQWidget());
    QTableWidgetItem *item = tablewidget->item(row, column);
    return item ? item->text().toStdString() : "";
}

void Table::setRowHeaderText(int row, std::string text)
{
    QTableWidget *tablewidget = static_cast<QTableWidget*>(getQWidget());
    return tablewidget->setVerticalHeaderItem(row, new QTableWidgetItem(QString::fromStdString(text)));
}

void Table::setColumnHeaderText(int column, std::string text)
{
    QTableWidget *tablewidget = static_cast<QTableWidget*>(getQWidget());
    return tablewidget->setHorizontalHeaderItem(column, new QTableWidgetItem(QString::fromStdString(text)));
}

void Table::setItemEditable(int row, int column, bool editable)
{
    QTableWidget *tablewidget = static_cast<QTableWidget*>(getQWidget());
    if(editable)
        tablewidget->item(row, column)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    else
        tablewidget->item(row, column)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

std::string Table::saveState()
{
    QTableWidget *tablewidget = static_cast<QTableWidget*>(getQWidget());
    QByteArray hhState = tablewidget->horizontalHeader()->saveState();
    QByteArray vhState = tablewidget->verticalHeader()->saveState();
    size_t len1 = hhState.length(), len2 = vhState.length();
    std::string hh(hhState.constData(), hhState.length()), vh(vhState.constData(), vhState.length());
    std::string state = boost::lexical_cast<std::string>(len1) + ":" + boost::lexical_cast<std::string>(len2) + ":" + hh + vh;
    return state;
}

bool Table::restoreState(std::string state)
{
    QTableWidget *tablewidget = static_cast<QTableWidget*>(getQWidget());
    size_t d1 = state.find(":"), d2 = state.find(":", d1 + 1);
    std::string slen1 = state.substr(0, d1), slen2 = state.substr(d1 + 1, d2 - d1);
    size_t len1 = std::stol(slen1), len2 = std::stol(slen2);
    const char *s = state.c_str() + d2 + 1;
    QByteArray hhState(s, len1), vhState(s + len1, len2);
    bool ret = true;
    ret = ret && tablewidget->horizontalHeader()->restoreState(hhState);
    ret = ret && tablewidget->verticalHeader()->restoreState(vhState);
    return ret;
}

void Table::setRowHeight(int row, int min_size, int max_size)
{
    QTableWidget *tablewidget = static_cast<QTableWidget*>(getQWidget());
    tablewidget->verticalHeader()->setMinimumSectionSize(min_size);
    tablewidget->verticalHeader()->setMaximumSectionSize(max_size);
}

void Table::setColumnWidth(int column, int min_size, int max_size)
{
    QTableWidget *tablewidget = static_cast<QTableWidget*>(getQWidget());
    tablewidget->horizontalHeader()->setMinimumSectionSize(min_size);
    tablewidget->horizontalHeader()->setMaximumSectionSize(max_size);
}

void Table::setSelection(int row, int column)
{
    QTableWidget *tablewidget = static_cast<QTableWidget*>(getQWidget());
    tablewidget->setCurrentCell(row, column);
}

