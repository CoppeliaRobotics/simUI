#include "Button.h"

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

    if(e->Attribute("text"))
        text = e->Attribute("text");
    else
        text = "???";

    if(e->Attribute("default") && strcmp(e->Attribute("default"), "true") == 0)
        defaulT = true;
    else
        defaulT = false;

    if(e->Attribute("onclick"))
        onclick = e->Attribute("onclick");
    else
        onclick = "";

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

