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

    autoRepeat = xmlutils::getAttrBool(e, "autoRepeat", false);

    autoRepeatDelay = xmlutils::getAttrInt(e, "autoRepeatDelay", 0);

    autoRepeatInterval = xmlutils::getAttrInt(e, "autoRepeatInterval", 200);

    onclick = xmlutils::getAttrStr(e, "onclick", "");
}

QWidget * Button::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QPushButton *button = new QPushButton(QString::fromStdString(text), parent);
    button->setEnabled(enabled);
    button->setVisible(visible);
    button->setStyleSheet(QString::fromStdString(style));
    button->setAutoDefault(false);
    button->setDefault(defaulT);
    button->setAutoRepeat(autoRepeat);
    button->setAutoRepeatDelay(autoRepeatDelay);
    button->setAutoRepeatInterval(autoRepeatInterval);
    QObject::connect(button, SIGNAL(released()), uiproxy, SLOT(onButtonClick()));
    setQWidget(button);
    setProxy(proxy);
    return button;
}

