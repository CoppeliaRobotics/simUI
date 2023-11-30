#include "Tabs.h"

#include "XMLUtils.h"

#include "UI.h"

#include <QWidget>
#include <QTabWidget>

Tab::Tab()
    : Widget("tab")
{
}

Tab::~Tab()
{
}

void Tab::parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(parent, widgets, e);

    if(!dynamic_cast<Tabs*>(parent))
    {
        throw std::runtime_error("tab must be placed in a tabs element");
    }

    title = xmlutils::getAttrStr(e, "title", "???");

    LayoutWidget::parse(this, parent, widgets, e);
}

QWidget * Tab::createQtWidget(Proxy *proxy, UI *ui, QWidget *parent)
{
    QWidget *tab = new QWidget(parent);
    tab->setEnabled(enabled);
    tab->setVisible(visible);
    tab->setStyleSheet(QString::fromStdString(style));
    LayoutWidget::createQtWidget(proxy, ui, tab);
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

void Tabs::parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(parent, widgets, e);

    for(tinyxml2::XMLElement *e1 = e->FirstChildElement(); e1; e1 = e1->NextSiblingElement())
    {
        Tab *tab = Widget::parse1<Tab>(this, widgets, e1);
        if(tab)
            tabs.push_back(tab);
    }
}

QWidget * Tabs::createQtWidget(Proxy *proxy, UI *ui, QWidget *parent)
{
    QTabWidget *tabwidget = new QTabWidget(parent);
    tabwidget->setEnabled(enabled);
    tabwidget->setVisible(visible);
    tabwidget->setStyleSheet(QString::fromStdString(style));
    for(std::vector<Tab*>::const_iterator it = tabs.begin(); it != tabs.end(); ++it)
    {
        QWidget *tab = (*it)->createQtWidget(proxy, ui, tabwidget);
        tabwidget->addTab(tab, QString::fromStdString((*it)->title));
    }
    setQWidget(tabwidget);
    setProxy(proxy);
    return tabwidget;
}

void Tabs::setCurrentTab(int index, bool suppressSignals)
{
    QTabWidget *qtabwidget = static_cast<QTabWidget*>(getQWidget());
    bool oldSignalsState = qtabwidget->blockSignals(suppressSignals);
    qtabwidget->setCurrentIndex(index);
    qtabwidget->blockSignals(oldSignalsState);
}

int Tabs::getCurrentTab()
{
    QTabWidget *qtabwidget = static_cast<QTabWidget*>(getQWidget());
    return qtabwidget->currentIndex();
}

