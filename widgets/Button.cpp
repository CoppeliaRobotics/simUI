#include "Button.h"

#include "XMLUtils.h"

#include "UIProxy.h"

#include <QPushButton>

Button::Button()
    : Widget()
{
}

Button::~Button()
{
}

const char * Button::name()
{
    return "button";
}

bool Button::parse(tinyxml2::XMLElement *e, std::vector<std::string>& errors)
{
    if(!Widget::parse(e, errors)) return false;

    text = xmlutils::getAttrStr(e, "text", "???");

    defaulT = xmlutils::getAttrBool(e, "default", false);

    onclick = xmlutils::getAttrStr(e, "onclick", "");

    return true;
}

QWidget * Button::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QPushButton *button = new QPushButton(QString::fromStdString(text), parent);
    button->setAutoDefault(false);
    button->setDefault(defaulT);
    QObject::connect(button, SIGNAL(released()), uiproxy, SLOT(onButtonClick()));
    setQWidget(button);
    setProxy(proxy);
    return button;
}

