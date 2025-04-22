#include "LayoutWidget.h"

#include "XMLUtils.h"

#include "Widget.h"
#include "Window.h"

#include "UI.h"

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

QWidget * Stretch::createQtWidget(Proxy *proxy, UI *ui, QWidget *parent)
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
    std::string layoutStr = xmlutils::getAttrStrEnum(e, "layout", "vbox", {"vbox", "hbox", "grid", "form", "none"});
    if(layoutStr == "vbox") layout = VBOX;
    else if(layoutStr == "hbox") layout = HBOX;
    else if(layoutStr == "grid") layout = GRID;
    else if(layoutStr == "form") layout = FORM;
    else if(layoutStr == "none") layout = NONE;
    else
    {
        throw std::range_error("invalid value '" + layoutStr + "' for attribute 'layout'");
    }

    std::vector<int> contentMarginsv = xmlutils::getAttrIntV(e, "content-margins", "7,7,7,7", 4, 4, ",");
    for(int i = 0; i < 4; i++) contentMargins[i] = contentMarginsv[i];

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
            if(w->geometry.isSet && layout != NONE)
                throw std::range_error("cannot set geometry with current layout");
            row.push_back(w);
        }
        catch(std::exception& ex)
        {
            children.push_back(row); // push widgets created until now so they won't leak
            throw std::range_error(ex.what());
        }

        if((layout == FORM && row.size() == 2) ||
                layout == VBOX ||
                layout == HBOX ||
                layout == NONE)
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

void LayoutWidget::createQtWidget(Proxy *proxy, UI *ui, QWidget *parent)
{
    switch(layout)
    {
    case VBOX:
    case HBOX:
    case NONE:
        {
            QBoxLayout *qlayout = 0;
            if(layout == VBOX) qlayout = new QVBoxLayout(parent);
            if(layout == HBOX) qlayout = new QHBoxLayout(parent);
            if(qlayout) qlayout->setContentsMargins(contentMargins[0], contentMargins[1], contentMargins[2], contentMargins[3]);
            for(std::vector< std::vector<Widget*> >::iterator it = children.begin(); it != children.end(); ++it)
            {
                Widget *w = (*it)[0];
                if(qlayout && dynamic_cast<Stretch*>(w))
                {
                    Stretch *stretch = static_cast<Stretch*>(w);
                    qlayout->addStretch(stretch->factor);
                    continue;
                }
                QWidget *qw = w->createQtWidget(proxy, ui, parent);
                if(qlayout)
                {
                    if(w->stretch == -1)
                        qlayout->addWidget(qw);
                    else
                        qlayout->addWidget(qw, w->stretch);
                }
                if(layout == NONE && w->geometry.isSet)
                {
                    qw->move(w->geometry.x, w->geometry.y);
                    qw->resize(w->geometry.width, w->geometry.height);
                }
            }
            if(qlayout) parent->setLayout(qlayout);
        }
        break;
    case GRID:
        {
            QGridLayout *qlayout = new QGridLayout(parent);
            qlayout->setContentsMargins(contentMargins[0], contentMargins[1], contentMargins[2], contentMargins[3]);
            int row = 0, col = 0;
            for(std::vector< std::vector<Widget*> >::iterator it = children.begin(); it != children.end(); ++it)
            {
                col = 0;
                for(std::vector<Widget*>::iterator it2 = it->begin(); it2 != it->end(); ++it2)
                {
                    Widget *w = *it2;
                    QWidget *qw = w->createQtWidget(proxy, ui, parent);
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
            qlayout->setContentsMargins(contentMargins[0], contentMargins[1], contentMargins[2], contentMargins[3]);
            for(std::vector< std::vector<Widget*> >::iterator it = children.begin(); it != children.end(); ++it)
            {
                Widget *w1 = (*it)[0], *w2 = (*it)[1];
                QWidget *qw1 = w1->createQtWidget(proxy, ui, parent);
                QWidget *qw2 = w2->createQtWidget(proxy, ui, parent);
                qlayout->addRow(qw1, qw2);
            }
            parent->setLayout(qlayout);
        }
        break;
    }
}

