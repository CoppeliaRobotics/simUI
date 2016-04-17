#include "Checkbox.h"

#include "XMLUtils.h"

#include "UIProxy.h"

#include <QCheckBox>

Checkbox::Checkbox()
    : Widget()
{
}

Checkbox::~Checkbox()
{
}

const char * Checkbox::name()
{
    return "checkbox";
}

bool Checkbox::parse(tinyxml2::XMLElement *e, std::vector<std::string>& errors)
{
    if(!Widget::parse(e, errors)) return false;

    text = xmlutils::getAttrStr(e, "text", "???");

    onchange = xmlutils::getAttrStr(e, "onchange", "");

    return true;
}

QWidget * Checkbox::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QCheckBox *checkbox = new QCheckBox(QString::fromStdString(text), parent);
    QObject::connect(checkbox, SIGNAL(stateChanged(int)), uiproxy, SLOT(onValueChange(int)));
    setQWidget(checkbox);
    setProxy(proxy);
    return checkbox;
}

