#include "Radiobutton.h"

#include "XMLUtils.h"

#include "UIProxy.h"

#include <QRadioButton>

Radiobutton::Radiobutton()
    : Widget("radiobutton")
{
}

Radiobutton::~Radiobutton()
{
}

void Radiobutton::parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(parent, widgets, e);

    text = xmlutils::getAttrStr(e, "text", "???");

    checked = xmlutils::getAttrBool(e, "checked", false);

    onclick = xmlutils::getAttrStr(e, "onclick", "");
}

QWidget * Radiobutton::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QRadioButton *button = new QRadioButton(QString::fromStdString(text), parent);
    button->setEnabled(enabled);
    button->setVisible(visible);
    button->setStyleSheet(QString::fromStdString(style));
    button->setChecked(checked);
    QObject::connect(button, SIGNAL(released()), uiproxy, SLOT(onButtonClick()));
    setQWidget(button);
    setProxy(proxy);
    return button;
}

