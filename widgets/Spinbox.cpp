#include "Spinbox.h"

#include "XMLUtils.h"

#include "UIProxy.h"

#include <QSpinBox>

Spinbox::Spinbox()
    : Widget("spinbox")
{
}

Spinbox::~Spinbox()
{
}

void Spinbox::parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(parent, widgets, e);

    minimum = xmlutils::getAttrInt(e, "minimum", 0);

    maximum = xmlutils::getAttrInt(e, "maximum", 100);

    prefix = xmlutils::getAttrStr(e, "prefix", "");

    suffix = xmlutils::getAttrStr(e, "suffix", "");

    step = xmlutils::getAttrInt(e, "step", 1);

    onchange = xmlutils::getAttrStr(e, "onchange", "");
}

QWidget * Spinbox::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QSpinBox *spinbox = new QSpinBox(parent);
    spinbox->setStyleSheet(QString::fromStdString(style));
    spinbox->setMinimum(minimum);
    spinbox->setMaximum(maximum);
    spinbox->setPrefix(QString::fromStdString(prefix));
    spinbox->setSuffix(QString::fromStdString(suffix));
    spinbox->setSingleStep(step);
    QObject::connect(spinbox, SIGNAL(valueChanged(int)), uiproxy, SLOT(onValueChange(int)));
    setQWidget(spinbox);
    setProxy(proxy);
    return spinbox;
}

