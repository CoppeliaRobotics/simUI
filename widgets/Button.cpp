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

    onclick = xmlutils::getAttrStr(e, "onclick", "");
}

QWidget * Button::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QPushButton *button = new QPushButton(QString::fromStdString(text), parent);
    button->setEnabled(enabled);
    button->setStyleSheet(QString::fromStdString(style));
    button->setAutoDefault(false);
    button->setDefault(defaulT);
    QObject::connect(button, SIGNAL(released()), uiproxy, SLOT(onButtonClick()));
    setQWidget(button);
    setProxy(proxy);
    return button;
}

