#include "Radiobutton.h"

#include "UIProxy.h"

#include <QRadioButton>

Radiobutton::Radiobutton()
    : Widget()
{
}

Radiobutton::~Radiobutton()
{
}

const char * Radiobutton::name()
{
    return "radiobutton";
}

bool Radiobutton::parse(tinyxml2::XMLElement *e, std::vector<std::string>& errors)
{
    if(!Widget::parse(e, errors)) return false;

    std::string tag(e->Value());
    if(tag != "radiobutton")
    {
        errors.push_back("element must be <radiobutton>");
        return false;
    }

    if(e->Attribute("text")) text = e->Attribute("text");
    else text = "";

    if(e->Attribute("onclick")) onclick = e->Attribute("onclick");
    else onclick = "";

    return true;
}

QWidget * Radiobutton::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QRadioButton *button = new QRadioButton(QString::fromStdString(text), parent);
    QObject::connect(button, SIGNAL(released()), uiproxy, SLOT(onButtonClick()));
    qwidget = button;
    Widget::widgetByQWidget[qwidget] = this;
    this->proxy = proxy;
    return button;
}

