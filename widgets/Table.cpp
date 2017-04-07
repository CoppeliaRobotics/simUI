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
                std::string itemName(e2->GetText());
                rows[rows.size()-1].push_back(itemName);
            }
        }
        else continue;
    }

    show_header = xmlutils::getAttrBool(e, "show-header", true);

    show_line_counter = xmlutils::getAttrBool(e, "show-line-counter", false);

    show_grid = xmlutils::getAttrBool(e, "show-grid", true);

    editable = xmlutils::getAttrBool(e, "editable", true);

    onselectionchange = xmlutils::getAttrStr(e, "onselectionchange", "");

    onchange = xmlutils::getAttrStr(e, "onchange", "");
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
            tablewidget->setItem(row, column, new QTableWidgetItem(QString::fromStdString(rows[row][column])));
        }
    }
    QObject::connect(tablewidget, SIGNAL(currentIndexChanged(int)), uiproxy, SLOT(onValueChange(int)));
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

