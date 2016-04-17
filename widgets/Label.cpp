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

bool Label::parse(tinyxml2::XMLElement *e, std::vector<std::string>& errors)
{
    if(!Widget::parse(e, errors)) return false;

    text = xmlutils::getAttrStr(e, "text", "");

    wordWrap = xmlutils::getAttrBool(e, "wordwrap", false);

    return true;
}

QWidget * Label::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QLabel *label = new QLabel(QString::fromStdString(text), parent);
    label->setWordWrap(wordWrap);
    setQWidget(label);
    setProxy(proxy);
    return label;
}

