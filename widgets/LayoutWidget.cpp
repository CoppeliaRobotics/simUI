#include "LayoutWidget.h"

#include "XMLUtils.h"

#include "Widget.h"
#include "Window.h"

#include "UIProxy.h"

#include <iostream>

#include <QBoxLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFormLayout>

#include <sstream>
#include <stdexcept>

Stretch::Stretch()
    : Widget("stretch")
{
}

void Stretch::parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(parent, widgets, e);

    LayoutWidget *layoutWidget = dynamic_cast<LayoutWidget*>(parent);
    if(!layoutWidget || !(layoutWidget->layout == VBOX || layoutWidget->layout == HBOX))
    {
        throw std::runtime_error("stretch must be placed in a widget with vbox/hbox layout");
    }
    
    factor = xmlutils::getAttrInt(e, "factor", 0);
}

QWidget * Stretch::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    throw std::runtime_error("stretch can be used only in VBox/HBox layouts");
}

LayoutWidget::~LayoutWidget()
{
    for(std::vector< std::vector<Widget*> >::iterator it = children.begin(); it != children.end(); ++it)
    {
        for(std::vector<Widget*>::iterator it2 = it->begin(); it2 != it->end(); ++it2)
        {
            delete *it2;
        }
    }
}

void LayoutWidget::parse(Widget *self, Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    std::string layoutStr = xmlutils::getAttrStr(e, "layout", "vbox");
    if(layoutStr == "vbox") layout = VBOX;
    else if(layoutStr == "hbox") layout = HBOX;
    else if(layoutStr == "grid") layout = GRID;
    else if(layoutStr == "form") layout = FORM;
    else
    {
        throw std::range_error("invalid value '" + layoutStr + "' for attribute 'layout'");
    }

    std::vector<Widget*> row;
    for(tinyxml2::XMLElement *e1 = e->FirstChildElement(); e1; e1 = e1->NextSiblingElement())
    {
        std::string tag1(e1->Value());

        // special elements:

        if(tag1 == "br" && layout == GRID)
        {
            children.push_back(row);
            row.clear();
            continue;
        }

        try
        {
            Widget *w = Widget::parseAny(self, widgets, e1);
            row.push_back(w);
        }
        catch(std::exception& ex)
        {
            children.push_back(row); // push widgets created until now so they won't leak
            throw std::range_error(ex.what());
        }

        if((layout == FORM && row.size() == 2) ||
                layout == VBOX ||
                layout == HBOX)
        {
            children.push_back(row);
            row.clear();
        }
    }
    if(row.size() > 0 && layout == GRID)
    {
        children.push_back(row);
        row.clear();
    }
    if(row.size() > 0)
    {
        children.push_back(row); // push widget created until now so they won't leak
        throw std::range_error("extra elements in layout");
    }
}

void LayoutWidget::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    switch(layout)
    {
    case VBOX:
    case HBOX:
        {
            QBoxLayout *qlayout;
            if(layout == VBOX) qlayout = new QVBoxLayout(parent);
            if(layout == HBOX) qlayout = new QHBoxLayout(parent);
            for(std::vector< std::vector<Widget*> >::iterator it = children.begin(); it != children.end(); ++it)
            {
                Widget *w = (*it)[0];
                if(dynamic_cast<Stretch*>(w))
                {
                    Stretch *stretch = static_cast<Stretch*>(w);
                    qlayout->addStretch(stretch->factor);
                    continue;
                }
                QWidget *qw = w->createQtWidget(proxy, uiproxy, parent);
                qlayout->addWidget(qw);
            }
            parent->setLayout(qlayout);
        }
        break;
    case GRID:
        {
            QGridLayout *qlayout = new QGridLayout(parent);
            int row = 0, col = 0;
            for(std::vector< std::vector<Widget*> >::iterator it = children.begin(); it != children.end(); ++it)
            {
                col = 0;
                for(std::vector<Widget*>::iterator it2 = it->begin(); it2 != it->end(); ++it2)
                {
                    Widget *w = *it2;
                    QWidget *qw = w->createQtWidget(proxy, uiproxy, parent);
                    qlayout->addWidget(qw, row, col);
                    col++;
                }
                row++;
            }
            parent->setLayout(qlayout);
        }
        break;
    case FORM:
        {
            QFormLayout *qlayout = new QFormLayout(parent);
            for(std::vector< std::vector<Widget*> >::iterator it = children.begin(); it != children.end(); ++it)
            {
                Widget *w1 = (*it)[0], *w2 = (*it)[1];
                QWidget *qw1 = w1->createQtWidget(proxy, uiproxy, parent);
                QWidget *qw2 = w2->createQtWidget(proxy, uiproxy, parent);
                qlayout->addRow(qw1, qw2);
            }
            parent->setLayout(qlayout);
        }
        break;
    }
}

