#include "Radiobutton.h"

#include "XMLUtils.h"

#include "UIProxy.h"

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

QWidget * Radiobutton::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QRadioButton *button = new QRadioButton(QString::fromStdString(text), parent);
    button->setEnabled(enabled);
    button->setVisible(visible);
    button->setStyleSheet(QString::fromStdString(style));
    button->setChecked(checked);
    button->setCheckable(checkable);
    button->setAutoExclusive(auto_exclusive);
    QObject::connect(button, &QRadioButton::released, uiproxy, &UIProxy::onButtonClick);
    setQWidget(button);
    setProxy(proxy);
    return button;
}

