#include "UIProxy.h"
#include "debug.h"

#include <QThread>
#include <QWidget>
#include <QPixmap>
#include <QImage>
#include <QLabel>
#include <QSlider>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QWidget>
#include <QGroupBox>
#include <QComboBox>
#include <QDialog>

#include "stubs.h"

using namespace tinyxml2;

// UIProxy is a singleton

UIProxy *UIProxy::instance = NULL;

UIProxy::UIProxy(QObject *parent)
    : QObject(parent)
{
}

UIProxy::~UIProxy()
{
    UIProxy::instance = NULL;
}

UIProxy * UIProxy::getInstance(QObject *parent)
{
    if(!UIProxy::instance)
    {
        UIProxy::instance = new UIProxy(parent);

        uiThread();

        DBG << "UIProxy(" << UIProxy::instance << ") constructed in thread " << QThread::currentThreadId() << std::endl;
    }
    return UIProxy::instance;
}

void UIProxy::destroyInstance()
{
    if(UIProxy::instance)
        delete UIProxy::instance;
}

void UIProxy::onCreate(Proxy *proxy)
{
    proxy->createQtWidget(this);
}

// The following slots are directly connected to Qt Widgets' signals.
// Here we look up the sender to find the corresponding Widget object,
// and emit a signal with the Widget object pointer.
//
// That signal will be connected to a slot in UIFunctions, such
// that the callback is called from the SIM thread.

void UIProxy::onButtonClick()
{
    QWidget *qwidget = dynamic_cast<QWidget*>(sender());
    if(qwidget)
    {
        Widget *widget = Widget::byQWidget(qwidget);
        if(widget)
        {
            emit buttonClick(widget);
        }
    }
}

void UIProxy::onValueChange(int value)
{
    QWidget *qwidget = dynamic_cast<QWidget*>(sender());
    if(qwidget)
    {
        Widget *widget = Widget::byQWidget(qwidget);
        if(widget)
        {
            emit valueChange(widget, value);
        }
    }
}

void UIProxy::onValueChange(double value)
{
    QWidget *qwidget = dynamic_cast<QWidget*>(sender());
    if(qwidget)
    {
        Widget *widget = Widget::byQWidget(qwidget);
        if(widget)
        {
            emit valueChange(widget, value);
        }
    }
}

void UIProxy::onValueChange(QString value)
{
    QWidget *qwidget = dynamic_cast<QWidget*>(sender());
    if(qwidget)
    {
        Widget *widget = Widget::byQWidget(qwidget);
        if(widget)
        {
            emit valueChange(widget, value);
        }
    }
}

void UIProxy::onEditingFinished()
{
    QLineEdit *qedit = dynamic_cast<QLineEdit*>(sender());

    if(qedit)
    {
        QString text = qedit->text();
        Edit *edit = dynamic_cast<Edit*>(Widget::byQWidget(qedit));

        if(edit)
        {
            emit editingFinished(edit, text);
        }
    }
}

// The following slots are wrappers for functions called from SIM thread
// which should instead execute in the UI thread.

void UIProxy::onDestroy(Proxy *proxy)
{
    DBG << "proxy=" << (void*)proxy << std::endl;

    if(!proxy) return;
    if(!proxy->ui) return;
    if(!proxy->ui->qwidget) return;

    proxy->ui->qwidget->deleteLater();
}

void UIProxy::onShowWindow(Window *window)
{
    DBG << "window=" << (void*)window << std::endl;

    if(!window) return;

    window->show();
}

void UIProxy::onHideWindow(Window *window)
{
    DBG << "window=" << (void*)window << std::endl;

    if(!window) return;

    window->hide();
}

void UIProxy::onSetPosition(Window *window, int x, int y)
{
    if(!window) return;

    window->getQWidget()->move(x, y);
}

void UIProxy::onSetImage(Image *image, const char *data, int w, int h)
{
    DBG << "image=" << (void*)image << ", data=" << (void*)data << ", w=" << w << ", h=" << h << std::endl;

    if(!image) return;
    if(!data) return;

    QImage::Format format = QImage::Format_RGB888;
    int bpp = 3; // bytes per pixel
    QPixmap pixmap = QPixmap::fromImage(QImage((unsigned char *)data, w, h, bpp * w, format));
    simReleaseBufferE((char *)data); // XXX: simReleaseBuffer should accept a const pointer
    QLabel *label = static_cast<QLabel*>(image->qwidget);
    label->setPixmap(pixmap);
    label->resize(pixmap.size());
}

void UIProxy::onSceneChange(Window *window, int oldSceneID, int newSceneID)
{
    DBG << "window=" << (void*)window << ", oldSceneID=" << oldSceneID << ", newSceneID" << newSceneID << std::endl;

    if(!window) return;

    window->onSceneChange(oldSceneID, newSceneID);
}

void UIProxy::onSetEnabled(Widget *widget, bool enabled)
{
    DBG << "widget=" << (void*)widget << ", enabled=" << enabled << std::endl;

    if(!widget) return;

    widget->getQWidget()->setEnabled(enabled);
}

void UIProxy::onSetEditValue(Edit *edit, std::string value, bool suppressSignals)
{
    QLineEdit *qedit = static_cast<QLineEdit*>(edit->getQWidget());
    bool oldSignalsState = qedit->blockSignals(suppressSignals);
    qedit->setText(QString::fromStdString(value));
    qedit->blockSignals(oldSignalsState);
}

void UIProxy::onSetSpinboxValue(Spinbox *spinbox_, double value, bool suppressSignals)
{
    QSpinBox *spinbox = dynamic_cast<QSpinBox*>(spinbox_->getQWidget());
    QDoubleSpinBox *doubleSpinbox = dynamic_cast<QDoubleSpinBox*>(spinbox_->getQWidget());
    if(spinbox)
    {
        bool oldSignalsState = spinbox->blockSignals(suppressSignals);
        spinbox->setValue(int(value));
        spinbox->blockSignals(oldSignalsState);
    }
    else if(doubleSpinbox)
    {
        bool oldSignalsState = doubleSpinbox->blockSignals(suppressSignals);
        doubleSpinbox->setValue(value);
        doubleSpinbox->blockSignals(oldSignalsState);
    }
    else static_cast<QSpinBox*>(spinbox_->getQWidget());
}

void UIProxy::onSetLabelText(Label *label, std::string text, bool suppressSignals)
{
    QLabel *qlabel = static_cast<QLabel*>(label->getQWidget());
    bool oldSignalsState = qlabel->blockSignals(suppressSignals);
    qlabel->setText(QString::fromStdString(text));
    qlabel->blockSignals(oldSignalsState);
}

void UIProxy::onSetSliderValue(Slider *slider, int value, bool suppressSignals)
{
    QSlider *qslider = static_cast<QSlider*>(slider->getQWidget());
    bool oldSignalsState = qslider->blockSignals(suppressSignals);
    qslider->setValue(value);
    qslider->blockSignals(oldSignalsState);
}

void UIProxy::onSetCheckboxValue(Checkbox *checkbox, int value, bool suppressSignals)
{
    QCheckBox *qcheckbox = static_cast<QCheckBox*>(checkbox->getQWidget());
    bool oldSignalsState = qcheckbox->blockSignals(suppressSignals);
    switch(value)
    {
    case 0: qcheckbox->setCheckState(Qt::Unchecked); break;
    case 1: qcheckbox->setCheckState(Qt::PartiallyChecked); break;
    case 2: qcheckbox->setCheckState(Qt::Checked); break;
    default: throw std::string("invalid checkbox value");
    }
    qcheckbox->blockSignals(oldSignalsState);
}

void UIProxy::onSetRadiobuttonValue(Radiobutton *radiobutton, int value, bool suppressSignals)
{
    QRadioButton *qradiobutton = static_cast<QRadioButton*>(radiobutton->getQWidget());
    bool oldSignalsState = qradiobutton->blockSignals(suppressSignals);
    switch(value)
    {
    case 0: qradiobutton->setChecked(false); break;
    case 1: qradiobutton->setChecked(true); break;
    default: throw std::string("invalid radiobutton value");
    }
    qradiobutton->blockSignals(oldSignalsState);
}

void UIProxy::onInsertComboboxItem(Combobox *combobox, int index, std::string text, bool suppressSignals)
{
    QComboBox *qcombobox = static_cast<QComboBox*>(combobox->getQWidget());
    bool oldSignalsState = qcombobox->blockSignals(suppressSignals);
    qcombobox->insertItem(index, QString::fromStdString(text));
    qcombobox->blockSignals(oldSignalsState);
}

void UIProxy::onRemoveComboboxItem(Combobox *combobox, int index, bool suppressSignals)
{
    QComboBox *qcombobox = static_cast<QComboBox*>(combobox->getQWidget());
    bool oldSignalsState = qcombobox->blockSignals(suppressSignals);
    qcombobox->removeItem(index);
    qcombobox->blockSignals(oldSignalsState);
}

