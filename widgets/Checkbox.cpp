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

