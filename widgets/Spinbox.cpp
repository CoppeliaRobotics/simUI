#include "Spinbox.h"

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

class XSpinBox;
class XDoubleSpinBox;

#include <QSpinBox>
#include <QDoubleSpinBox>

struct RMBDragFeature;

class XSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit XSpinBox(QWidget *parent = nullptr);

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:

private:
    RMBDragFeature *rmbDrag = nullptr;
};

class XDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
public:
    explicit XDoubleSpinBox(QWidget *parent = nullptr);

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:

private:
    RMBDragFeature *rmbDrag = nullptr;
};

#include <QMouseEvent>
#include <QStyleOptionSpinBox>
#include <QTimer>
#include <QDebug>

struct RMBDragFeature
{
private:
    QAbstractSpinBox *spinbox;
    int mouseStartPosY = 0;
    int startValueInt = 0;
    double startValueDouble = 0;
    Qt::ContextMenuPolicy previousContextMenuPolicy;

public:
    static RMBDragFeature * hitTest(QAbstractSpinBox *spinbox, QStyleOptionSpinBox *opt, QMouseEvent *event)
    {
        opt->subControls = QStyle::SC_All;
        QStyle::SubControl hoverControl = spinbox->style()->hitTestComplexControl(QStyle::CC_SpinBox, opt, event->pos(), spinbox);
        if(hoverControl == QStyle::SC_SpinBoxUp || hoverControl == QStyle::SC_SpinBoxDown)
        {
            return new RMBDragFeature(spinbox, event);
        }
        return nullptr;
    }

    RMBDragFeature(QAbstractSpinBox *spinbox, QMouseEvent *event)
        : spinbox(spinbox)
    {
        mouseStartPosY = event->pos().y();
        previousContextMenuPolicy = spinbox->contextMenuPolicy();
        spinbox->setContextMenuPolicy(Qt::PreventContextMenu);

        if(auto s = dynamic_cast<QSpinBox*>(spinbox))
            startValueInt = s->value();
        else if(auto s = dynamic_cast<QDoubleSpinBox*>(spinbox))
            startValueDouble = s->value();
    }

    void move(QMouseEvent *event)
    {
        spinbox->setCursor(Qt::SizeVerCursor);

        const float multiplier = 0.5;
        int valueOffset = int((mouseStartPosY - event->pos().y()) * multiplier);

        if(auto s = dynamic_cast<QSpinBox*>(spinbox))
            s->setValue(startValueInt + valueOffset * s->singleStep());
        else if(auto s = dynamic_cast<QDoubleSpinBox*>(spinbox))
            s->setValue(startValueDouble + valueOffset * s->singleStep());
    }

    ~RMBDragFeature()
    {
        spinbox->unsetCursor();
        auto s = spinbox;
        auto pcmc = previousContextMenuPolicy;
        QTimer::singleShot(500, [s,pcmc] {s->setContextMenuPolicy(pcmc);});
    }
};

XSpinBox::XSpinBox(QWidget *parent) : QSpinBox(parent)
{
}

void XSpinBox::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
    {
        QStyleOptionSpinBox opt;
        initStyleOption(&opt);
        if((rmbDrag = RMBDragFeature::hitTest(this, &opt, event))) return;
    }

    QSpinBox::mousePressEvent(event);
}

void XSpinBox::mouseMoveEvent(QMouseEvent *event)
{
    if(rmbDrag) rmbDrag->move(event);
}

void XSpinBox::mouseReleaseEvent(QMouseEvent *event)
{
    QSpinBox::mouseReleaseEvent(event);

    if(rmbDrag)
    {
        delete rmbDrag;
        rmbDrag = nullptr;
        return;
    }
}

XDoubleSpinBox::XDoubleSpinBox(QWidget *parent) : QDoubleSpinBox(parent)
{
}

void XDoubleSpinBox::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
    {
        QStyleOptionSpinBox opt;
        initStyleOption(&opt);
        if((rmbDrag = RMBDragFeature::hitTest(this, &opt, event))) return;
    }

    QDoubleSpinBox::mousePressEvent(event);
}

void XDoubleSpinBox::mouseMoveEvent(QMouseEvent *event)
{
    if(rmbDrag) rmbDrag->move(event);
}

void XDoubleSpinBox::mouseReleaseEvent(QMouseEvent *event)
{
    QDoubleSpinBox::mouseReleaseEvent(event);

    if(rmbDrag)
    {
        delete rmbDrag;
        rmbDrag = nullptr;
        return;
    }
}

QWidget * Spinbox::createQtWidget(Proxy *proxy, UI *ui, QWidget *parent)
{
    if(float_)
    {
        QDoubleSpinBox *spinbox = new XDoubleSpinBox(parent);
        spinbox->setEnabled(enabled);
        spinbox->setVisible(visible);
        spinbox->setStyleSheet(QString::fromStdString(style));
        spinbox->setValue(value);
        spinbox->setMinimum(minimum);
        spinbox->setMaximum(maximum);
        spinbox->setPrefix(QString::fromStdString(prefix));
        spinbox->setSuffix(QString::fromStdString(suffix));
        spinbox->setSingleStep(step);
        spinbox->setDecimals(decimals > -1 ? decimals : 6);
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
        spinbox->setValue(int(value));
        spinbox->setMinimum(int(minimum));
        spinbox->setMaximum(int(maximum));
        spinbox->setPrefix(QString::fromStdString(prefix));
        spinbox->setSuffix(QString::fromStdString(suffix));
        spinbox->setSingleStep(int(step));
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

