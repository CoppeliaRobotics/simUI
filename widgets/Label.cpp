#include "Label.h"

#include "UIProxy.h"

#include <QLabel>

Label::Label()
    : Widget()
{
}

Label::~Label()
{
}

bool Label::parse(tinyxml2::XMLElement *e, std::vector<std::string>& errors)
{
    if(!Widget::parse(e, errors)) return false;

    std::string tag(e->Value());
    if(tag != "label")
    {
        errors.push_back("element must be <label>");
        return false;
    }

    if(e->Attribute("text")) text = e->Attribute("text");
    else text = "";

    return true;
}

QWidget * Label::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QLabel *label = new QLabel(QString::fromStdString(text), parent);
    qwidget = label;
    Widget::widgetByQWidget[qwidget] = this;
    this->proxy = proxy;
    return label;
}

