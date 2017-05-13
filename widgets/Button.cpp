#include "Button.h"

#include "XMLUtils.h"

#include "UIProxy.h"

#include <QPushButton>

Button::Button()
    : Widget("button")
{
}

Button::~Button()
{
}

void Button::parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(parent, widgets, e);

    text = xmlutils::getAttrStr(e, "text", "???");

    defaulT = xmlutils::getAttrBool(e, "default", false);

    auto_repeat = xmlutils::getAttrBool(e, "auto-repeat", false);

    auto_repeat_delay = xmlutils::getAttrInt(e, "auto-repeat-delay", 0);

    auto_repeat_interval = xmlutils::getAttrInt(e, "auto-repeat-interval", 200);

    checkable = xmlutils::getAttrBool(e, "checkable", false);

    auto_exclusive = xmlutils::getAttrBool(e, "auto-exclusive", false);

    onclick = xmlutils::getAttrStr(e, "on-click", "");
}

QWidget * Button::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QPushButton *button = new QPushButton(QString::fromStdString(text), parent);
    button->setEnabled(enabled);
    button->setVisible(visible);
    button->setStyleSheet(QString::fromStdString(style));
    button->setAutoDefault(false);
    button->setDefault(defaulT);
    button->setAutoRepeat(auto_repeat);
    button->setAutoRepeatDelay(auto_repeat_delay);
    button->setAutoRepeatInterval(auto_repeat_interval);
    button->setCheckable(checkable);
    button->setAutoExclusive(auto_exclusive);
    QObject::connect(button, &QPushButton::released, uiproxy, &UIProxy::onButtonClick);
    setQWidget(button);
    setProxy(proxy);
    return button;
}

