#include "Checkbox.h"

#include "XMLUtils.h"

#include "UIProxy.h"

#include <QCheckBox>

Checkbox::Checkbox()
    : Widget("checkbox")
{
}

Checkbox::~Checkbox()
{
}

void Checkbox::parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(parent, widgets, e);

    text = xmlutils::getAttrStr(e, "text", "???");

    checked = xmlutils::getAttrBool(e, "checked", false);

    checkable = xmlutils::getAttrBool(e, "checkable", true);

    auto_exclusive = xmlutils::getAttrBool(e, "auto-exclusive", false);

    onchange = xmlutils::getAttrStr(e, "on-change", "");
}

QWidget * Checkbox::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QCheckBox *checkbox = new QCheckBox(QString::fromStdString(text), parent);
    checkbox->setEnabled(enabled);
    checkbox->setVisible(visible);
    checkbox->setStyleSheet(QString::fromStdString(style));
    checkbox->setChecked(checked);
    checkbox->setCheckable(checkable);
    checkbox->setAutoExclusive(auto_exclusive);
    QObject::connect(checkbox, &QCheckBox::stateChanged, uiproxy, &UIProxy::onValueChangeInt);
    setQWidget(checkbox);
    setProxy(proxy);
    return checkbox;
}

Qt::CheckState Checkbox::convertValueFromInt(int value)
{
    switch(value)
    {
    case 0: return Qt::Unchecked;
    case 1: return Qt::PartiallyChecked;
    case 2: return Qt::Checked;
    default: throw std::range_error("invalid checkbox value: must be 0, 1 or 2");
    }
}

int Checkbox::convertValueToInt(Qt::CheckState value)
{
    switch(value)
    {
    case Qt::Unchecked: return 0;
    case Qt::PartiallyChecked: return 1;
    case Qt::Checked: return 2;
    default: throw std::range_error("invalid checkbox value: must be Unchecked, PartiallyChecked or Checked");
    }
}

void Checkbox::setValue(Qt::CheckState value, bool suppressSignals)
{
    QCheckBox *qcheckbox = static_cast<QCheckBox*>(getQWidget());
    bool oldSignalsState = qcheckbox->blockSignals(suppressSignals);
    qcheckbox->setCheckState(value);
    qcheckbox->blockSignals(oldSignalsState);
}

Qt::CheckState Checkbox::getValue()
{
    QCheckBox *qcheckbox = static_cast<QCheckBox*>(getQWidget());
    return qcheckbox->checkState();
}

