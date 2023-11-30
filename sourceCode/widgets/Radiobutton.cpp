#include "Radiobutton.h"

#include "XMLUtils.h"

#include "UI.h"

#include <QRadioButton>

Radiobutton::Radiobutton()
    : Widget("radiobutton")
{
}

Radiobutton::~Radiobutton()
{
}

void Radiobutton::parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(parent, widgets, e);

    text = xmlutils::getAttrStr(e, "text", "???");

    checked = xmlutils::getAttrBool(e, "checked", false);

    checkable = xmlutils::getAttrBool(e, "checkable", true);

    auto_exclusive = xmlutils::getAttrBool(e, "auto-exclusive", true);

    onclick = xmlutils::getAttrStr(e, "on-click", "");
}

QWidget * Radiobutton::createQtWidget(Proxy *proxy, UI *ui, QWidget *parent)
{
    QRadioButton *button = new QRadioButton(QString::fromStdString(text), parent);
    button->setEnabled(enabled);
    button->setVisible(visible);
    button->setStyleSheet(QString::fromStdString(style));
    button->setCheckable(checkable);
    button->setAutoExclusive(auto_exclusive);
    button->setChecked(checked);
    QObject::connect(button, &QRadioButton::released, ui, &UI::onButtonClick);
    setQWidget(button);
    setProxy(proxy);
    return button;
}

bool Radiobutton::convertValueFromInt(int value)
{
    switch(value)
    {
    case 0: return false;
    case 1: return true;
    default: throw std::range_error("invalid radiobutton value: must be 0 or 1");
    }
}

int Radiobutton::convertValueToInt(bool value)
{
    return value ? 1 : 0;
}

void Radiobutton::setValue(bool value, bool suppressSignals)
{
    QRadioButton *qradiobutton = static_cast<QRadioButton*>(getQWidget());
    bool oldSignalsState = qradiobutton->blockSignals(suppressSignals);
    qradiobutton->setChecked(value);
    qradiobutton->blockSignals(oldSignalsState);
}

bool Radiobutton::getValue()
{
    QRadioButton *qradiobutton = static_cast<QRadioButton*>(getQWidget());
    return qradiobutton->isChecked();
}

