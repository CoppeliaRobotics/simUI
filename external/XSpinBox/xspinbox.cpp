#include "xspinbox.h"

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
    static bool contextMenuPolicyToRestore_;
    static Qt::ContextMenuPolicy contextMenuPolicyToRestore;

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
        if(!contextMenuPolicyToRestore_)
        {
            contextMenuPolicyToRestore_ = true;
            contextMenuPolicyToRestore = spinbox->contextMenuPolicy();
        }
        spinbox->setContextMenuPolicy(Qt::PreventContextMenu);

        if(auto s = dynamic_cast<QSpinBox*>(spinbox))
            startValueInt = s->value();
        else if(auto s = dynamic_cast<QDoubleSpinBox*>(spinbox))
            startValueDouble = s->value();

        spinbox->setCursor(Qt::SizeVerCursor);
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
        QTimer::singleShot(500, [s] {s->setContextMenuPolicy(contextMenuPolicyToRestore);});
    }
};

bool RMBDragFeature::contextMenuPolicyToRestore_ = false;
Qt::ContextMenuPolicy RMBDragFeature::contextMenuPolicyToRestore = Qt::PreventContextMenu;

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
