#include "Table.h"

#include "XMLUtils.h"

#include "UIProxy.h"
#include "debug.h"

#include <iostream>

#include <QTableWidget>

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
                header.push_back(itemName);
            }
        }
        else if(tag1 == "row")
        {
            rows.resize(rows.size()+1);
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

    show_header = xmlutils::getAttrBool(e, "show-header", true);

    show_line_counter = xmlutils::getAttrBool(e, "show-line-counter", false);

    show_grid = xmlutils::getAttrBool(e, "show-grid", true);

    editable = xmlutils::getAttrBool(e, "editable", true);

    onCellActivate = xmlutils::getAttrStr(e, "oncellactivate", "");

    onSelectionChange = xmlutils::getAttrStr(e, "onselectionchange", "");

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
    tablewidget->horizontalHeader()->setVisible(show_header);
    tablewidget->verticalHeader()->setVisible(show_line_counter);
    tablewidget->setShowGrid(show_grid);
    QStringList qtheader;
    for(size_t i = 0; i < header.size(); i++)
    {
        qtheader << QString::fromStdString(header[i]);
    }
    tablewidget->setHorizontalHeaderLabels(qtheader);
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
    QObject::connect(tablewidget, &QTableWidget::itemSelectionChanged, uiproxy, &UIProxy::onSelectionChange);
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
    return tablewidget->item(row, column)->text().toStdString();
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

