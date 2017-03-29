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

QWidget *UIProxy::vrepMainWindow = NULL;

simFloat UIProxy::wheelZoomFactor = 1.0;

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
        UIProxy::vrepMainWindow = (QWidget *)simGetMainWindow(1);
        simGetFloatParameter(sim_floatparam_mouse_wheel_zoom_factor, &UIProxy::wheelZoomFactor);

        uiThread(); // we remember this currentThreadId as the "UI" thread

        DBG << "UIProxy(" << UIProxy::instance << ") constructed in thread " << QThread::currentThreadId() << std::endl;
    }
    return UIProxy::instance;
}

void UIProxy::destroyInstance()
{
    DBG << "[enter]" << std::endl;

    if(UIProxy::instance)
    {
        delete UIProxy::instance;

        DBG << "destroyed UIProxy instance" << std::endl;
    }

    DBG << "[leave]" << std::endl;
}

void UIProxy::onCreate(Proxy *proxy)
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    proxy->createQtWidget(this);

    DBG << "[leave]" << std::endl;
}

// The following slots are directly connected to Qt Widgets' signals.
// Here we look up the sender to find the corresponding Widget object,
// and emit a signal with the Widget object pointer.
//
// That signal will be connected to a slot in UIFunctions, such
// that the callback is called from the SIM thread.

void UIProxy::onButtonClick()
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    QWidget *qwidget = dynamic_cast<QWidget*>(sender());
    if(qwidget)
    {
        Widget *widget = Widget::byQWidget(qwidget);
        if(widget)
        {
            emit buttonClick(widget);
        }
    }

    DBG << "[leave]" << std::endl;
}

void UIProxy::onValueChange(int value)
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    QWidget *qwidget = dynamic_cast<QWidget*>(sender());
    if(qwidget)
    {
        Widget *widget = Widget::byQWidget(qwidget);
        if(widget)
        {
            emit valueChange(widget, value);
        }
    }

    DBG << "[leave]" << std::endl;
}

void UIProxy::onValueChange(double value)
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    QWidget *qwidget = dynamic_cast<QWidget*>(sender());
    if(qwidget)
    {
        Widget *widget = Widget::byQWidget(qwidget);
        if(widget)
        {
            emit valueChange(widget, value);
        }
    }

    DBG << "[leave]" << std::endl;
}

void UIProxy::onValueChange(QString value)
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    QWidget *qwidget = dynamic_cast<QWidget*>(sender());
    if(qwidget)
    {
        Widget *widget = Widget::byQWidget(qwidget);
        if(widget)
        {
            emit valueChange(widget, value);
        }
    }

    DBG << "[leave]" << std::endl;
}

void UIProxy::onEditingFinished()
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

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

    DBG << "[leave]" << std::endl;
}

void UIProxy::onPlottableClick(QCPAbstractPlottable *plottable, int index, QMouseEvent *event)
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    if(QWidget *qwidget = dynamic_cast<QWidget*>(sender()))
    {
        if(Plot *plot = dynamic_cast<Plot*>(Widget::byQWidget(qwidget)))
        {
            std::string name = plottable->name().toStdString();

            float x = NAN, y = NAN;
            if(QCPGraph *graph = dynamic_cast<QCPGraph*>(plottable))
            {
                QCPGraphData d = *graph->data()->at(index);
                x = d.key;
                y = d.value;
            }
            else if(QCPCurve *curve = dynamic_cast<QCPCurve*>(plottable))
            {
                QCPCurveData d = *curve->data()->at(index);
                x = d.key;
                y = d.value;
            }

            emit plottableClick(plot, name, index, x, y);
        }
    }

    DBG << "[leave]" << std::endl;
}

void UIProxy::onLegendClick(QCPLegend *legend, QCPAbstractLegendItem *item, QMouseEvent *event)
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    if(QWidget *qwidget = dynamic_cast<QWidget*>(sender()))
    {
        if(Plot *plot = dynamic_cast<Plot*>(Widget::byQWidget(qwidget)))
        {
            if(QCPPlottableLegendItem *item1 = dynamic_cast<QCPPlottableLegendItem*>(item))
            {
                QCPAbstractPlottable *plottable = item1->plottable();

                std::string name = plottable->name().toStdString();

                emit legendClick(plot, name);
            }
        }
    }

    DBG << "[leave]" << std::endl;
}

// The following slots are wrappers for functions called from SIM thread
// which should instead execute in the UI thread.

void UIProxy::onDestroy(Proxy *proxy)
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    DBG << "proxy=" << (void*)proxy << std::endl;

    if(!proxy)
    {
        DBG << "WARNING: proxy is NULL" << std::endl;
        return;
    }
    if(!proxy->ui)
    {
        DBG << "WARNING: proxy->ui is NULL" << std::endl;
        return;
    }
    if(!proxy->ui->qwidget)
    {
        DBG << "WARNING: proxy->ui->qwidget is NULL" << std::endl;
        return;
    }

    DBG << "calling proxy->ui->qwidget->deleteLater()..." << std::endl;
    proxy->ui->qwidget->deleteLater();

    DBG << "[leave]" << std::endl;
}

void UIProxy::onShowWindow(Window *window)
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    DBG << "window=" << (void*)window << std::endl;

    if(!window) return;

    window->show();

    DBG << "[leave]" << std::endl;
}

void UIProxy::onHideWindow(Window *window)
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    DBG << "window=" << (void*)window << std::endl;

    if(!window) return;

    window->hide();

    DBG << "[leave]" << std::endl;
}

void UIProxy::onSetPosition(Window *window, int x, int y)
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    if(!window) return;

    window->getQWidget()->move(x, y);

    DBG << "[leave]" << std::endl;
}

void UIProxy::onSetSize(Window *window, int w, int h)
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    if(!window) return;

    window->getQWidget()->resize(w, h);

    DBG << "[leave]" << std::endl;
}

void UIProxy::onSetTitle(Window *window, std::string title)
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    if(!window) return;

    static_cast<QDialog*>(window->getQWidget())->setWindowTitle(QString::fromStdString(title));

    DBG << "[leave]" << std::endl;
}

void UIProxy::onSetImage(Image *image, const char *data, int w, int h)
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

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

    DBG << "[leave]" << std::endl;
}

void UIProxy::onSceneChange(Window *window, int oldSceneID, int newSceneID)
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    DBG << "window=" << (void*)window << ", oldSceneID=" << oldSceneID << ", newSceneID" << newSceneID << std::endl;

    if(!window) return;

    window->onSceneChange(oldSceneID, newSceneID);

    DBG << "[leave]" << std::endl;
}

void UIProxy::onSetEnabled(Widget *widget, bool enabled)
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    DBG << "widget=" << (void*)widget << ", enabled=" << enabled << std::endl;

    if(!widget) return;

    widget->getQWidget()->setEnabled(enabled);

    DBG << "[leave]" << std::endl;
}

void UIProxy::onSetEditValue(Edit *edit, std::string value, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    QLineEdit *qedit = static_cast<QLineEdit*>(edit->getQWidget());
    bool oldSignalsState = qedit->blockSignals(suppressSignals);
    qedit->setText(QString::fromStdString(value));
    qedit->blockSignals(oldSignalsState);

    DBG << "[leave]" << std::endl;
}

void UIProxy::onSetSpinboxValue(Spinbox *spinbox_, double value, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

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

    DBG << "[leave]" << std::endl;
}

void UIProxy::onSetLabelText(Label *label, std::string text, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    QLabel *qlabel = static_cast<QLabel*>(label->getQWidget());
    bool oldSignalsState = qlabel->blockSignals(suppressSignals);
    qlabel->setText(QString::fromStdString(text));
    qlabel->blockSignals(oldSignalsState);

    DBG << "[leave]" << std::endl;
}

void UIProxy::onSetSliderValue(Slider *slider, int value, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    QSlider *qslider = static_cast<QSlider*>(slider->getQWidget());
    bool oldSignalsState = qslider->blockSignals(suppressSignals);
    qslider->setValue(value);
    qslider->blockSignals(oldSignalsState);

    DBG << "[leave]" << std::endl;
}

void UIProxy::onSetCheckboxValue(Checkbox *checkbox, int value, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

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

    DBG << "[leave]" << std::endl;
}

void UIProxy::onSetRadiobuttonValue(Radiobutton *radiobutton, int value, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    QRadioButton *qradiobutton = static_cast<QRadioButton*>(radiobutton->getQWidget());
    bool oldSignalsState = qradiobutton->blockSignals(suppressSignals);
    switch(value)
    {
    case 0: qradiobutton->setChecked(false); break;
    case 1: qradiobutton->setChecked(true); break;
    default: throw std::string("invalid radiobutton value");
    }
    qradiobutton->blockSignals(oldSignalsState);

    DBG << "[leave]" << std::endl;
}

void UIProxy::onInsertComboboxItem(Combobox *combobox, int index, std::string text, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    QComboBox *qcombobox = static_cast<QComboBox*>(combobox->getQWidget());
    bool oldSignalsState = qcombobox->blockSignals(suppressSignals);
    qcombobox->insertItem(index, QString::fromStdString(text));
    qcombobox->blockSignals(oldSignalsState);

    DBG << "[leave]" << std::endl;
}

void UIProxy::onRemoveComboboxItem(Combobox *combobox, int index, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    QComboBox *qcombobox = static_cast<QComboBox*>(combobox->getQWidget());
    bool oldSignalsState = qcombobox->blockSignals(suppressSignals);
    qcombobox->removeItem(index);
    qcombobox->blockSignals(oldSignalsState);

    DBG << "[leave]" << std::endl;
}

void UIProxy::onSetComboboxItems(Combobox *combobox, QStringList &items, int index, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    QComboBox *qcombobox = static_cast<QComboBox*>(combobox->getQWidget());
    bool oldSignalsState = qcombobox->blockSignals(suppressSignals);
    qcombobox->clear();
    qcombobox->addItems(items);
    qcombobox->setCurrentIndex(index);
    qcombobox->blockSignals(oldSignalsState);

    DBG << "[leave]" << std::endl;
}

void UIProxy::onSetComboboxSelectedIndex(Combobox *combobox, int index, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    QComboBox *qcombobox = static_cast<QComboBox*>(combobox->getQWidget());
    bool oldSignalsState = qcombobox->blockSignals(suppressSignals);
    qcombobox->setCurrentIndex(index);
    qcombobox->blockSignals(oldSignalsState);

    DBG << "[leave]" << std::endl;
}

void UIProxy::onSetCurrentTab(Tabs *tabs, int index, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    QTabWidget *qtabwidget = static_cast<QTabWidget*>(tabs->getQWidget());
    bool oldSignalsState = qtabwidget->blockSignals(suppressSignals);
    qtabwidget->setCurrentIndex(index);
    qtabwidget->blockSignals(oldSignalsState);

    DBG << "[leave]" << std::endl;
}

void UIProxy::onSetWidgetVisibility(Widget *widget, bool visible)
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    DBG << "widget=" << (void*)widget << ", visible=" << visible << std::endl;

    if(!widget) return;

    widget->getQWidget()->setVisible(visible);

    DBG << "[leave]" << std::endl;
}

void UIProxy::onReplot(Plot *plot)
{
    plot->replot(true);
}

void UIProxy::onAddCurve(Plot *plot, int type, std::string name, std::vector<int> color, int style, curve_options *opts)
{
    plot->addCurve(type, name, color, style, opts);
}

void UIProxy::onAddCurveTimePoints(Plot *plot, std::string name, std::vector<double> x, std::vector<double> y)
{
    plot->addTimeData(name, x, y);
}

void UIProxy::onAddCurveXYPoints(Plot *plot, std::string name, std::vector<double> t, std::vector<double> x, std::vector<double> y)
{
    plot->addXYData(name, t, x, y);
}

void UIProxy::onClearCurve(Plot *plot, std::string name)
{
    plot->clearCurve(name);
}

void UIProxy::onRemoveCurve(Plot *plot, std::string name)
{
    plot->removeCurve(name);
}

void UIProxy::onSetPlotRanges(Plot *plot, double xmin, double xmax, double ymin, double ymax)
{
    QCustomPlot *qplot = static_cast<QCustomPlot*>(plot->getQWidget());
    qplot->xAxis->setRange(xmin, xmax);
    qplot->yAxis->setRange(ymin, ymax);
}

void UIProxy::onSetPlotXRange(Plot *plot, double xmin, double xmax)
{
    QCustomPlot *qplot = static_cast<QCustomPlot*>(plot->getQWidget());
    qplot->xAxis->setRange(xmin, xmax);
}

void UIProxy::onSetPlotYRange(Plot *plot, double ymin, double ymax)
{
    QCustomPlot *qplot = static_cast<QCustomPlot*>(plot->getQWidget());
    qplot->yAxis->setRange(ymin, ymax);
}

void UIProxy::onGrowPlotRanges(Plot *plot, double xmin, double xmax, double ymin, double ymax)
{
    QCustomPlot *qplot = static_cast<QCustomPlot*>(plot->getQWidget());
    QCPRange xrange = qplot->xAxis->range();
    qplot->xAxis->setRange(xrange.lower - xmin, xrange.upper + xmax);
    QCPRange yrange = qplot->yAxis->range();
    qplot->yAxis->setRange(yrange.lower - ymin, yrange.upper + ymax);
}

void UIProxy::onGrowPlotXRange(Plot *plot, double xmin, double xmax)
{
    QCustomPlot *qplot = static_cast<QCustomPlot*>(plot->getQWidget());
    QCPRange xrange = qplot->xAxis->range();
    qplot->xAxis->setRange(xrange.lower - xmin, xrange.upper + xmax);
}

void UIProxy::onGrowPlotYRange(Plot *plot, double ymin, double ymax)
{
    QCustomPlot *qplot = static_cast<QCustomPlot*>(plot->getQWidget());
    QCPRange yrange = qplot->yAxis->range();
    qplot->yAxis->setRange(yrange.lower - ymin, yrange.upper + ymax);
}

void UIProxy::onSetPlotLabels(Plot *plot, std::string x, std::string y)
{
    QCustomPlot *qplot = static_cast<QCustomPlot*>(plot->getQWidget());
    qplot->xAxis->setLabel(QString::fromStdString(x));
    qplot->yAxis->setLabel(QString::fromStdString(y));
}

void UIProxy::onSetPlotXLabel(Plot *plot, std::string label)
{
    QCustomPlot *qplot = static_cast<QCustomPlot*>(plot->getQWidget());
    qplot->xAxis->setLabel(QString::fromStdString(label));
}

void UIProxy::onSetPlotYLabel(Plot *plot, std::string label)
{
    QCustomPlot *qplot = static_cast<QCustomPlot*>(plot->getQWidget());
    qplot->yAxis->setLabel(QString::fromStdString(label));
}

void UIProxy::onRescaleAxes(Plot *plot, std::string name, bool onlyEnlargeX, bool onlyEnlargeY)
{
    plot->rescaleAxes(name, onlyEnlargeX, onlyEnlargeY);
}

void UIProxy::onRescaleAxesAll(Plot *plot, bool onlyEnlargeX, bool onlyEnlargeY)
{
    plot->rescaleAxesAll(onlyEnlargeX, onlyEnlargeY);
}

void UIProxy::onSetMouseOptions(Plot *plot, bool panX, bool panY, bool zoomX, bool zoomY)
{
    plot->setMouseOptions(panX, panY, zoomX, zoomY);
}

void UIProxy::onSetLegendVisibility(Plot *plot, bool visible)
{
    plot->setLegendVisibility(visible);
}
