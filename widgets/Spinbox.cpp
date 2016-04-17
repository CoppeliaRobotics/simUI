#include "Spinbox.h"

#include "XMLUtils.h"

#include "UIProxy.h"

#include <QSpinBox>

Spinbox::Spinbox()
    : Widget()
{
}

Spinbox::~Spinbox()
{
}

const char * Spinbox::name()
{
    return "spinbox";
}

bool Spinbox::parse(tinyxml2::XMLElement *e, std::vector<std::string>& errors)
{
    if(!Widget::parse(e, errors)) return false;

    minimum = xmlutils::getAttrInt(e, "minimum", 0);

    maximum = xmlutils::getAttrInt(e, "maximum", 100);

    prefix = xmlutils::getAttrStr(e, "prefix", "");

    suffix = xmlutils::getAttrStr(e, "suffix", "");

    step = xmlutils::getAttrInt(e, "step", 1);

    onchange = xmlutils::getAttrStr(e, "onchange", "");

    return true;
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

