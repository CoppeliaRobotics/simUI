#include "Spinbox.h"

#include "XSpinBox/xspinbox.h"

#include "XMLUtils.h"
#include "UI.h"

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

    value = xmlutils::getAttrDouble(e, "value", 0);

    minimum = xmlutils::getAttrDouble(e, "minimum", 0);

    maximum = xmlutils::getAttrDouble(e, "maximum", 100);

    prefix = xmlutils::getAttrStr(e, "prefix", "");

    suffix = xmlutils::getAttrStr(e, "suffix", "");

    step = xmlutils::getAttrDouble(e, "step", 1);

    decimals = xmlutils::getAttrInt(e, "decimals", -1);

    onchange = xmlutils::getAttrStr(e, "on-change", "");

    bool detectedFloat = isFloat(minimum) || isFloat(maximum) || isFloat(step) || decimals > -1;
    float_ = xmlutils::getAttrBool(e, "float", detectedFloat);
}

QWidget * Spinbox::createQtWidget(Proxy *proxy, UI *ui, QWidget *parent)
{
    if(float_)
    {
        QDoubleSpinBox *spinbox = new XDoubleSpinBox(parent);
        spinbox->setEnabled(enabled);
        spinbox->setVisible(visible);
        spinbox->setStyleSheet(QString::fromStdString(style));
        spinbox->setMinimum(minimum);
        spinbox->setMaximum(maximum);
        spinbox->setPrefix(QString::fromStdString(prefix));
        spinbox->setSuffix(QString::fromStdString(suffix));
        spinbox->setSingleStep(step);
        spinbox->setDecimals(decimals > -1 ? decimals : 6);
        spinbox->setValue(value);
        QObject::connect(spinbox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), ui, &UI::onValueChangeDouble);
        setQWidget(spinbox);
        setProxy(proxy);
        return spinbox;
    }
    else
    {
        QSpinBox *spinbox = new XSpinBox(parent);
        spinbox->setEnabled(enabled);
        spinbox->setVisible(visible);
        spinbox->setStyleSheet(QString::fromStdString(style));
        spinbox->setMinimum(int(minimum));
        spinbox->setMaximum(int(maximum));
        spinbox->setPrefix(QString::fromStdString(prefix));
        spinbox->setSuffix(QString::fromStdString(suffix));
        spinbox->setSingleStep(int(step));
        spinbox->setValue(int(value));
        QObject::connect(spinbox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), ui, &UI::onValueChangeInt);
        setQWidget(spinbox);
        setProxy(proxy);
        return spinbox;
    }
}

void Spinbox::setValue(double value, bool suppressSignals)
{
    QWidget *qwidget = getQWidget();
    bool oldSignalsState = qwidget->blockSignals(suppressSignals);
    if(QSpinBox *spinbox = dynamic_cast<QSpinBox*>(qwidget))
    {
        spinbox->setValue(int(value));
    }
    else if(QDoubleSpinBox *doubleSpinbox = dynamic_cast<QDoubleSpinBox*>(qwidget))
    {
        doubleSpinbox->setValue(value);
    }
    qwidget->blockSignals(oldSignalsState);
}

double Spinbox::getValue()
{
    if(QSpinBox *spinbox = dynamic_cast<QSpinBox*>(getQWidget()))
    {
        return spinbox->value();
    }
    else if(QDoubleSpinBox *doubleSpinbox = dynamic_cast<QDoubleSpinBox*>(getQWidget()))
    {
        return doubleSpinbox->value();
    }
    else
    {
        return NAN;
    }
}

