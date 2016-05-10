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

void Spinbox::parse(std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(widgets, e);

    minimum = xmlutils::getAttrInt(e, "minimum", 0);

    maximum = xmlutils::getAttrInt(e, "maximum", 100);

    prefix = xmlutils::getAttrStr(e, "prefix", "");

    suffix = xmlutils::getAttrStr(e, "suffix", "");

    step = xmlutils::getAttrInt(e, "step", 1);

    onchange = xmlutils::getAttrStr(e, "onchange", "");
}

QWidget * Spinbox::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QSpinBox *slider = new QSpinBox(parent);
    slider->setMinimum(minimum);
    slider->setMaximum(maximum);
    slider->setPrefix(QString::fromStdString(prefix));
    slider->setSuffix(QString::fromStdString(suffix));
    slider->setSingleStep(step);
    QObject::connect(slider, SIGNAL(valueChanged(int)), uiproxy, SLOT(onValueChange(int)));
    setQWidget(slider);
    setProxy(proxy);
    return slider;
}

