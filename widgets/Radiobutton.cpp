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

void Radiobutton::parse(tinyxml2::XMLElement *e)
{
    Widget::parse(e);

    text = xmlutils::getAttrStr(e, "text", "???");

    onclick = xmlutils::getAttrStr(e, "onclick", "");
}

QWidget * Radiobutton::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QRadioButton *button = new QRadioButton(QString::fromStdString(text), parent);
    QObject::connect(button, SIGNAL(released()), uiproxy, SLOT(onButtonClick()));
    setQWidget(button);
    setProxy(proxy);
    return button;
}

