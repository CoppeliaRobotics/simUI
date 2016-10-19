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

static inline bool isFloat(double x)
{
    return std::fabs(x - std::floor(x)) > 1e-6;
}

void Spinbox::parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(parent, widgets, e);

    minimum = xmlutils::getAttrDouble(e, "minimum", 0);

    maximum = xmlutils::getAttrDouble(e, "maximum", 100);

    prefix = xmlutils::getAttrStr(e, "prefix", "");

    suffix = xmlutils::getAttrStr(e, "suffix", "");

    step = xmlutils::getAttrDouble(e, "step", 1);

    onchange = xmlutils::getAttrStr(e, "onchange", "");

    bool detectedFloat = isFloat(minimum) || isFloat(maximum) || isFloat(step);
    float_ = xmlutils::getAttrBool(e, "float", detectedFloat);
}

QWidget * Spinbox::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    if(float_)
    {
        QDoubleSpinBox *spinbox = new QDoubleSpinBox(parent);
        spinbox->setEnabled(enabled);
        spinbox->setVisible(visible);
        spinbox->setStyleSheet(QString::fromStdString(style));
        spinbox->setMinimum(minimum);
        spinbox->setMaximum(maximum);
        spinbox->setPrefix(QString::fromStdString(prefix));
        spinbox->setSuffix(QString::fromStdString(suffix));
        spinbox->setSingleStep(step);
        QObject::connect(spinbox, SIGNAL(valueChanged(double)), uiproxy, SLOT(onValueChange(double)));
        setQWidget(spinbox);
        setProxy(proxy);
        return spinbox;
    }
    else
    {
        QSpinBox *spinbox = new QSpinBox(parent);
        spinbox->setEnabled(enabled);
        spinbox->setVisible(visible);
        spinbox->setStyleSheet(QString::fromStdString(style));
        spinbox->setMinimum(int(minimum));
        spinbox->setMaximum(int(maximum));
        spinbox->setPrefix(QString::fromStdString(prefix));
        spinbox->setSuffix(QString::fromStdString(suffix));
        spinbox->setSingleStep(int(step));
        QObject::connect(spinbox, SIGNAL(valueChanged(int)), uiproxy, SLOT(onValueChange(int)));
        setQWidget(spinbox);
        setProxy(proxy);
        return spinbox;
    }
}

