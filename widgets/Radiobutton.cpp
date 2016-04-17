#include "Radiobutton.h"

#include "XMLUtils.h"

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

    text = xmlutils::getAttrStr(e, "text", "???");

    onclick = xmlutils::getAttrStr(e, "onclick", "");

    return true;
}

QWidget * Radiobutton::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QRadioButton *button = new QRadioButton(QString::fromStdString(text), parent);
    QObject::connect(button, SIGNAL(released()), uiproxy, SLOT(onButtonClick()));
    setQWidget(button);
    setProxy(proxy);
    return button;
}

