#include "Combobox.h"

#include "XMLUtils.h"

#include "UIProxy.h"

#include <iostream>
#include <boost/foreach.hpp>

#include <QComboBox>

Combobox::Combobox()
    : Widget("combobox")
{
}

Combobox::~Combobox()
{
}

void Combobox::parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(parent, widgets, e);

    for(tinyxml2::XMLElement *e1 = e->FirstChildElement(); e1; e1 = e1->NextSiblingElement())
    {
        std::string tag1(e1->Value());
        if(tag1 != "item") continue;
        std::string itemName(e1->GetText());
        items.push_back(itemName);
    }

    onchange = xmlutils::getAttrStr(e, "on-change", "");
}

QWidget * Combobox::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QComboBox *combobox = new QComboBox(parent);
    combobox->setEnabled(enabled);
    combobox->setVisible(visible);
    combobox->setStyleSheet(QString::fromStdString(style));
    for(std::vector<std::string>::const_iterator it = items.begin(); it != items.end(); ++it)
    {
        combobox->addItem(QString::fromStdString(*it));
    }
    QObject::connect(combobox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), uiproxy, &UIProxy::onValueChangeInt);
    setQWidget(combobox);
    setProxy(proxy);
    return combobox;
}

void Combobox::insertItem(int index, std::string text, bool suppressSignals)
{
    QComboBox *qcombobox = static_cast<QComboBox*>(getQWidget());
    bool oldSignalsState = qcombobox->blockSignals(suppressSignals);
    qcombobox->insertItem(index, QString::fromStdString(text));
    qcombobox->blockSignals(oldSignalsState);
}

void Combobox::removeItem(int index, bool suppressSignals)
{
    QComboBox *qcombobox = static_cast<QComboBox*>(getQWidget());
    bool oldSignalsState = qcombobox->blockSignals(suppressSignals);
    qcombobox->removeItem(index);
    qcombobox->blockSignals(oldSignalsState);
}

void Combobox::setItems(std::vector<std::string> items, int index, bool suppressSignals)
{
    QComboBox *qcombobox = static_cast<QComboBox*>(getQWidget());
    bool oldSignalsState = qcombobox->blockSignals(suppressSignals);
    qcombobox->clear();
    QStringList qitems;
    BOOST_FOREACH(std::string &item, items)
        qitems.push_back(QString::fromStdString(item));
    qcombobox->addItems(qitems);
    qcombobox->setCurrentIndex(index);
    qcombobox->blockSignals(oldSignalsState);
}

std::vector<std::string> Combobox::getItems()
{
    QComboBox *qcombobox = static_cast<QComboBox*>(getQWidget());
    std::vector<std::string> ret;
    for(int i = 0; i < qcombobox->count(); i++)
        ret.push_back(qcombobox->itemText(i).toStdString());
    return ret;
}

void Combobox::setSelectedIndex(int index, bool suppressSignals)
{
    QComboBox *qcombobox = static_cast<QComboBox*>(getQWidget());
    bool oldSignalsState = qcombobox->blockSignals(suppressSignals);
    qcombobox->setCurrentIndex(index);
    qcombobox->blockSignals(oldSignalsState);
}

int Combobox::getSelectedIndex()
{
    QComboBox *qcombobox = static_cast<QComboBox*>(getQWidget());
    return qcombobox->currentIndex();
}

int Combobox::count()
{
    QComboBox *qcombobox = static_cast<QComboBox*>(getQWidget());
    return qcombobox->count();
}

std::string Combobox::itemText(int index)
{
    QComboBox *qcombobox = static_cast<QComboBox*>(getQWidget());
    return qcombobox->itemText(index).toStdString();
}

