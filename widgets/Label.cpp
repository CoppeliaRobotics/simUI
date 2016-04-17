#include "Label.h"

#include "XMLUtils.h"

#include "UIProxy.h"

#include <QLabel>

Label::Label()
    : Widget()
{
}

Label::~Label()
{
}

const char * Label::name()
{
    return "label";
}

void Label::parse(tinyxml2::XMLElement *e)
{
    Widget::parse(e);

    text = xmlutils::getAttrStr(e, "text", "");

    wordWrap = xmlutils::getAttrBool(e, "wordwrap", false);
}

QWidget * Label::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QLabel *label = new QLabel(QString::fromStdString(text), parent);
    label->setWordWrap(wordWrap);
    setQWidget(label);
    setProxy(proxy);
    return label;
}

