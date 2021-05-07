#ifndef XSPINBOX_H_INCLUDED
#define XSPINBOX_H_INCLUDED

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

#endif // XSPINBOX_H_INCLUDED
