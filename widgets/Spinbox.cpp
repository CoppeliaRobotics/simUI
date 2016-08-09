#include "Spinbox.h"

#include "XMLUtils.h"
#include "debug.h"
#include "UIProxy.h"

#include <cmath>

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

    minimum = xmlutils::getAttrDouble(e, "minimum", 0);

    maximum = xmlutils::getAttrDouble(e, "maximum", 100);

    prefix = xmlutils::getAttrStr(e, "prefix", "");

    suffix = xmlutils::getAttrStr(e, "suffix", "");

    step = xmlutils::getAttrInt(e, "step", 1);

    onchange = xmlutils::getAttrStr(e, "onchange", "");

    bool detectedFloat = std::fabs(minimum - std::floor(minimum)) > 1e-6 ||
        std::fabs(maximum - std::floor(maximum)) > 1e-6;
    float_ = xmlutils::getAttrBool(e, "float", detectedFloat);
}

QWidget * Spinbox::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    if(float_)
    {
        QDoubleSpinBox *spinbox = new QDoubleSpinBox(parent);
        spinbox->setEnabled(enabled);
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
    else
    {
        QSpinBox *spinbox = new QSpinBox(parent);
        spinbox->setEnabled(enabled);
        spinbox->setStyleSheet(QString::fromStdString(style));
        spinbox->setMinimum(int(minimum));
        spinbox->setMaximum(int(maximum));
        spinbox->setPrefix(QString::fromStdString(prefix));
        spinbox->setSuffix(QString::fromStdString(suffix));
        spinbox->setSingleStep(step);
        QObject::connect(spinbox, SIGNAL(valueChanged(int)), uiproxy, SLOT(onValueChange(int)));
        setQWidget(spinbox);
        setProxy(proxy);
        return spinbox;
    }
}

