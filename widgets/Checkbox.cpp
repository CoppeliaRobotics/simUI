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

    onchange = xmlutils::getAttrStr(e, "onchange", "");
}

QWidget * Checkbox::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QCheckBox *checkbox = new QCheckBox(QString::fromStdString(text), parent);
    checkbox->setEnabled(enabled);
    checkbox->setVisible(visible);
    checkbox->setStyleSheet(QString::fromStdString(style));
    checkbox->setChecked(checked);
    QObject::connect(checkbox, SIGNAL(stateChanged(int)), uiproxy, SLOT(onValueChange(int)));
    setQWidget(checkbox);
    setProxy(proxy);
    return checkbox;
}

