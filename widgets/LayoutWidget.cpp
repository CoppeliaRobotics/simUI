#include "LayoutWidget.h"
#include "Widget.h"

#include "UIProxy.h"

#include <iostream>

#include <QBoxLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFormLayout>

LayoutWidget::~LayoutWidget()
{
#ifdef DEBUG
    std::cerr << "LayoutWidget::~LayoutWidget()" << std::endl;
#endif

    for(std::vector< std::vector<Widget*> >::iterator it = children.begin(); it != children.end(); ++it)
    {
        for(std::vector<Widget*>::iterator it2 = it->begin(); it2 != it->end(); ++it2)
        {
            delete *it2;
        }
    }
}

bool LayoutWidget::parse(tinyxml2::XMLElement *e, std::vector<std::string>& errors)
{
    if(e->Attribute("layout"))
    {
        std::string layoutName(e->Attribute("layout"));
        if(layoutName == "vbox") layout = VBOX;
        else if(layoutName == "hbox") layout = HBOX;
        else if(layoutName == "grid") layout = GRID;
        else if(layoutName == "form") layout = FORM;
        else
        {
            errors.push_back("invalid value '" + layoutName + "' for attribute 'layout'");
            return false;
        }
    }
    else layout = VBOX;

    std::vector<Widget*> row;
    for(tinyxml2::XMLElement *e1 = e->FirstChildElement(); e1; e1 = e1->NextSiblingElement())
    {
        std::string tag1(e1->Value());

        if(tag1 == "br" && layout == GRID)
        {
            children.push_back(row);
            row.clear();
            continue;
        }

        Widget *w = Widget::parseAny(e1, errors);
        if(!w)
        {
            children.push_back(row); // push widget created until now so they won't leak
            return false;
        }
        row.push_back(w);

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
        errors.push_back("extra elements in layout");
        children.push_back(row); // push widget created until now so they won't leak
        return false;
    }

    return true;
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
                QWidget *w = (*it)[0]->createQtWidget(proxy, uiproxy, parent);
                qlayout->addWidget(w);
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
                    QWidget *w = (*it2)->createQtWidget(proxy, uiproxy, parent);
                    qlayout->addWidget(w, row, col);
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
                QWidget *w1 = (*it)[0]->createQtWidget(proxy, uiproxy, parent);
                QWidget *w2 = (*it)[1]->createQtWidget(proxy, uiproxy, parent);
                qlayout->addRow(w1, w2);
            }
            parent->setLayout(qlayout);
        }
        break;
    }
}

