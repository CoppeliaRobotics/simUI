#include "Tabs.h"

#include "XMLUtils.h"

#include "UIProxy.h"

#include <QWidget>
#include <QTabWidget>

Tab::Tab()
    : Widget("tab")
{
}

Tab::~Tab()
{
}

void Tab::parse(tinyxml2::XMLElement *e)
{
    Widget::parse(e);

    title = xmlutils::getAttrStr(e, "title", "???");

    LayoutWidget::parse(e);
}

QWidget * Tab::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QWidget *tab = new QWidget(parent);
    LayoutWidget::createQtWidget(proxy, uiproxy, tab);
    setQWidget(tab);
    setProxy(proxy);
    return tab;
}

Tabs::Tabs()
    : Widget("tabs")
{
}

Tabs::~Tabs()
{
    for(std::vector<Tab*>::const_iterator it = tabs.begin(); it != tabs.end(); ++it)
    {
        delete *it;
    }
}

void Tabs::parse(tinyxml2::XMLElement *e)
{
    Widget::parse(e);

    for(tinyxml2::XMLElement *e1 = e->FirstChildElement(); e1; e1 = e1->NextSiblingElement())
    {
        Tab *tab = Widget::parse1<Tab>(e1);
        if(tab)
            tabs.push_back(tab);
    }
}

QWidget * Tabs::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QTabWidget *tabwidget = new QTabWidget(parent);
    for(std::vector<Tab*>::const_iterator it = tabs.begin(); it != tabs.end(); ++it)
    {
        QWidget *tab = (*it)->createQtWidget(proxy, uiproxy, tabwidget);
        tabwidget->addTab(tab, QString::fromStdString((*it)->title));
    }
    setQWidget(tabwidget);
    setProxy(proxy);
    return tabwidget;
}

