#include "UI.h"
#include "SIM.h"

#include <QThread>
#include <QMessageBox>
#include <QFileDialog>
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
#include <QTreeWidget>
#include <QTextBrowser>

#include <simPlusPlus/Lib.h>
#include "stubs.h"

using namespace tinyxml2;

// UI is a singleton

UI *UI::instance = NULL;

QWidget *UI::simMainWindow = NULL;

double UI::wheelZoomFactor = 1.0;

UI::UI(QObject *parent)
    : QObject(parent)
{
}

UI::~UI()
{
    UI::instance = NULL;
}

UI * UI::getInstance(QObject *parent)
{
    if(!UI::instance)
    {
        UI::instance = new UI(parent);
        UI::simMainWindow = (QWidget *)sim::getMainWindow(1);

        uiThread(); // we remember this currentThreadId as the "UI" thread

        sim::addLog(sim_verbosity_debug, "UI(%x) constructed in thread %s", UI::instance, QThread::currentThreadId());

        SIM::getInstance()->connectSignals();
    }
    return UI::instance;
}

void UI::destroyInstance()
{
    TRACE_FUNC;

    if(UI::instance)
    {
        delete UI::instance;

        sim::addLog(sim_verbosity_debug, "destroyed UI instance");
    }
}

void UI::onMsgBox(int type, int buttons, std::string title, std::string message, int *result)
{
    ASSERT_THREAD(UI);

    QString qtitle = QString::fromStdString(title);
    QString qmessage = QString::fromStdString(message);

    QMessageBox msgBox;
#ifdef MAC_SIM
    msgBox.setText(qtitle);
    msgBox.setInformativeText(qmessage);
#else
    msgBox.setWindowTitle(qtitle);
    msgBox.setText(qmessage);
#endif
    switch(type)
    {
    case simui_msgbox_type_info:
        msgBox.setIcon(QMessageBox::Information);
        break;
    case simui_msgbox_type_question:
        msgBox.setIcon(QMessageBox::Question);
        break;
    case simui_msgbox_type_warning:
        msgBox.setIcon(QMessageBox::Warning);
        break;
    case simui_msgbox_type_critical:
        msgBox.setIcon(QMessageBox::Critical);
        break;
    }
    switch(buttons)
    {
    case simui_msgbox_buttons_ok:
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        break;
    case simui_msgbox_buttons_yesno:
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        break;
    case simui_msgbox_buttons_yesnocancel:
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        break;
    case simui_msgbox_buttons_okcancel:
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        break;
    }
    int ret = msgBox.exec();
    switch(ret)
    {
    case QMessageBox::Ok:
        *result = simui_msgbox_result_ok;
        break;
    case QMessageBox::Cancel:
        *result = simui_msgbox_result_cancel;
        break;
    case QMessageBox::Yes:
        *result = simui_msgbox_result_yes;
        break;
    case QMessageBox::No:
        *result = simui_msgbox_result_no;
        break;
    }
}

void UI::onFileDialog(int type, std::string title, std::string startPath, std::string initName, std::string extName, std::string ext, bool native, std::vector<std::string> *result)
{
    ASSERT_THREAD(UI);

    QString qtitle = QString::fromStdString(title);
    QString qstartPath = QString::fromStdString(startPath);
    QString qinitName = QString::fromStdString(initName);
    if(qinitName != "")
    {
        if(qstartPath.length() > 0)
        {
            QChar c = qstartPath[qstartPath.length() - 1];
            if(c != '/' && c != '\\') qstartPath += "/";
        }
        qstartPath += qinitName;
    }
    QString qextName = QString::fromStdString(extName);
    QString qext = QString::fromStdString(ext);
    QStringList exts = qext.split(";");
    QString filter = qextName + " (";
    for(int i = 0; i < exts.length(); i++)
        filter += QString("%1*.%2").arg(i ? " " : "").arg(exts[i]);
    filter += ")";
    QFileDialog::Options opts;
    if(!native) opts |= QFileDialog::DontUseNativeDialog;
    QString file;
    QStringList files;

    switch(type)
    {
    case simui_filedialog_type_load:
        file = QFileDialog::getOpenFileName(nullptr, qtitle, qstartPath, filter, nullptr, opts);
        result->push_back(file.toStdString());
        break;
    case simui_filedialog_type_load_multiple:
        files = QFileDialog::getOpenFileNames(nullptr, qtitle, qstartPath, filter, nullptr, opts);
        for(auto file : files)
            result->push_back(file.toStdString());
        break;
    case simui_filedialog_type_save:
        file = QFileDialog::getSaveFileName(nullptr, qtitle, qstartPath, filter, nullptr, opts);
        result->push_back(file.toStdString());
        break;
    case simui_filedialog_type_folder:
        file = QFileDialog::getExistingDirectory(nullptr, qtitle, qstartPath, opts | QFileDialog::ShowDirsOnly);
        result->push_back(file.toStdString());
        break;
    }
}

void UI::onColorDialog(std::vector<float> initColor, std::string title, bool showAlphaChannel, bool native, std::vector<float> *result)
{
    ASSERT_THREAD(UI);

    QString qtitle = QString::fromStdString(title);
    QColor cinitColor;
    cinitColor.setRgbF(initColor[0], initColor[1], initColor[2]);
    if(initColor.size() == 4) cinitColor.setAlphaF(initColor[3]);
    if(!cinitColor.isValid()) cinitColor = Qt::white;

    QColorDialog::ColorDialogOptions opts;
    if(showAlphaChannel) opts |= QColorDialog::ShowAlphaChannel;
    if(!native) opts |= QColorDialog::DontUseNativeDialog;

    QColor cresult = QColorDialog::getColor(cinitColor, nullptr, qtitle, opts);
    if(cresult.isValid())
    {
        result->push_back(cresult.redF());
        result->push_back(cresult.greenF());
        result->push_back(cresult.blueF());
        if(showAlphaChannel) result->push_back(cresult.alphaF());
    }
}

void UI::onInputDialog(std::string initValue, std::string label, std::string title, bool *ok, std::string *result)
{
    ASSERT_THREAD(UI);

    *result = QInputDialog::getText(nullptr, QString::fromStdString(title), QString::fromStdString(label), QLineEdit::Normal, QString::fromStdString(initValue), ok, Qt::WindowFlags(), Qt::ImhNone).toStdString();
}

void UI::onCreate(Proxy *proxy)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    proxy->createQtWidget(this);
}

// The following slots are directly connected to Qt Widgets' signals.
// Here we look up the sender to find the corresponding Widget object,
// and emit a signal with the Widget object pointer.
//
// That signal will be connected to a slot in SIM, such
// that the callback is called from the SIM thread.

#if WIDGET_BUTTON || WIDGET_RADIOBUTTON
void UI::onButtonClick()
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

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
#endif

#if WIDGET_LABEL
void UI::onLinkActivated(const QString &link)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    QWidget *qwidget = dynamic_cast<QWidget*>(sender());
    if(qwidget)
    {
        Widget *widget = Widget::byQWidget(qwidget);
        if(widget)
        {
            emit linkActivated(widget, link);
        }
    }
}
#endif

void UI::onValueChangeInt(int value)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    QWidget *qwidget = dynamic_cast<QWidget*>(sender());
    if(qwidget)
    {
        Widget *widget = Widget::byQWidget(qwidget);
        if(widget)
        {
            emit valueChangeInt(widget, value);
        }
    }
}

void UI::onValueChangeDouble(double value)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    QWidget *qwidget = dynamic_cast<QWidget*>(sender());
    if(qwidget)
    {
        Widget *widget = Widget::byQWidget(qwidget);
        if(widget)
        {
            emit valueChangeDouble(widget, value);
        }
    }
}

void UI::onValueChangeString(QString value)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    QWidget *qwidget = dynamic_cast<QWidget*>(sender());
    if(qwidget)
    {
        Widget *widget = Widget::byQWidget(qwidget);
        if(widget)
        {
            emit valueChangeString(widget, value);
        }
    }
}

#if WIDGET_EDIT
void UI::onEditingFinished()
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

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
#endif

#if WIDGET_PLOT
void UI::onPlottableClick(QCPAbstractPlottable *plottable, int index, QMouseEvent *event)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

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
}

void UI::onLegendClick(QCPLegend *legend, QCPAbstractLegendItem *item, QMouseEvent *event)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

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
}
#endif

#if WIDGET_TABLE
void UI::onCellActivate(int row, int col)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    QTableWidget *qwidget = dynamic_cast<QTableWidget*>(sender());

    if(qwidget)
    {
        QString text = qwidget->item(row, col)->text();
        Table *table = dynamic_cast<Table*>(Widget::byQWidget(qwidget));
        if(table)
        {
            emit cellActivate(table, row, col, text.toStdString());
        }
    }
}

void UI::onTableSelectionChange()
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    QTableWidget *qwidget = dynamic_cast<QTableWidget*>(sender());

    if(qwidget)
    {
        QList<QModelIndex> indexes = qwidget->selectionModel()->selectedIndexes();
        int row = -1, column = -1;
        if(indexes.size() >= 1)
        {
            row = indexes[0].row();
            column = indexes[0].column();
        }
        if(indexes.size() > 1)
        {
            if(indexes[1].row() == row) column = -1;
            if(indexes[1].column() == column) row = -1;
        }
        Table *table = dynamic_cast<Table*>(Widget::byQWidget(qwidget));
        if(table)
        {
            emit tableSelectionChange(table, row, column);
        }
    }
}
#endif

#if WIDGET_TREE
void UI::onTreeSelectionChange()
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    QTreeWidget *qwidget = dynamic_cast<QTreeWidget*>(sender());

    if(qwidget)
    {
        QList<QTreeWidgetItem*> s = qwidget->selectedItems();
        int id = 0;
        Tree *tree = dynamic_cast<Tree*>(Widget::byQWidget(qwidget));
        if(s.size() > 0 && tree)
        {
            // FIXME: implement inverted index for this operation
            for(std::map<int, QTreeWidgetItem*>::iterator it = tree->widgetItemById.begin(); it != tree->widgetItemById.end(); ++it)
                if(it->second == s[0])
                    id = it->first;
        }
        if(tree)
        {
            emit treeSelectionChange(tree, id);
        }
    }
}
#endif

#if WIDGET_IMAGE || WIDGET_SVG
void UI::onMouseEvent(Widget *widget, int type, bool shift, bool control, int x, int y)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    emit mouseEvent(widget, type, shift, control, x, y);
}
#endif

#if WIDGET_TEXTBROWSER
void UI::onTextChanged()
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    if(QTextBrowser *qtextbrowser = dynamic_cast<QTextBrowser*>(sender()))
    {
        if(TextBrowser *textbrowser = dynamic_cast<TextBrowser*>(Widget::byQWidget(qtextbrowser)))
        {
            std::string text = textbrowser->getText();
            emit valueChangeString(textbrowser, QString::fromStdString(text));
        }
    }
}

void UI::onAnchorClicked(const QUrl &link)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    if(QTextBrowser *qtextbrowser = dynamic_cast<QTextBrowser*>(sender()))
    {
        if(TextBrowser *textbrowser = dynamic_cast<TextBrowser*>(Widget::byQWidget(qtextbrowser)))
        {
            emit linkActivated(textbrowser, link.url());
        }
    }
}
#endif

#if WIDGET_SCENE3D
void UI::onViewCenterChanged(const QVector3D &viewCenter)
{
    sim::addLog(sim_verbosity_debug, "View center changed: %f, %f, %f", viewCenter.x(), viewCenter.y(), viewCenter.z());
}

void UI::onPositionChanged(const QVector3D &position)
{
    sim::addLog(sim_verbosity_debug, "Position changed: %f, %f, %f", position.x(), position.y(), position.z());
}

void UI::onScene3DObjectClicked(Qt3DRender::QPickEvent *pick)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    if(Qt3DRender::QObjectPicker *obj = dynamic_cast<Qt3DRender::QObjectPicker*>(sender()))
    {
        Scene3D *scene3d = Scene3D::nodeScene(obj);
        if(scene3d)
        {
            int objId = scene3d->nodeId(obj);
            if(objId)
            {
                emit scene3DObjectClick(scene3d, objId);
            }
        }
        else
        {
            sim::addLog(sim_verbosity_debug, "received QPickEvent but sender() is not associated with a Scene3D widget");
        }
    }
    else
    {
        sim::addLog(sim_verbosity_debug, "received QPickEvent but sender() is not a QObjectPicker");
    }
}
#endif

// The following slots are wrappers for functions called from SIM thread
// which should instead execute in the UI thread.

void UI::onDestroy(Proxy *proxy)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    sim::addLog(sim_verbosity_debug, "proxy=%x",  proxy);

    if(!proxy)
    {
        sim::addLog(sim_verbosity_debug, "WARNING: proxy is NULL");
        return;
    }
    if(!proxy->window)
    {
        sim::addLog(sim_verbosity_debug, "WARNING: proxy->window is NULL");
        return;
    }
    if(!proxy->window->qwidget)
    {
        sim::addLog(sim_verbosity_debug, "WARNING: proxy->window->qwidget is NULL");
        return;
    }

    sim::addLog(sim_verbosity_debug, "deleting proxy->window->qwidget...");
    delete proxy->window->qwidget;
}

void UI::onSetStyleSheet(Widget *widget, std::string styleSheet)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    widget->getQWidget()->setStyleSheet(QString::fromStdString(styleSheet));
}

#if WIDGET_BUTTON
void UI::onSetButtonText(Button *button, std::string text)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    button->setText(text);
}

void UI::onSetButtonPressed(Button *button, bool pressed)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    button->setPressed(pressed);
}
#endif

void UI::onShowWindow(Window *window)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    sim::addLog(sim_verbosity_debug, "window=%x", window);

    if(!window) return;

    window->show();
}

void UI::onAdjustSize(Window *window)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    sim::addLog(sim_verbosity_debug, "window=%x", window);

    if(!window) return;

    window->adjustSize();
}

void UI::onHideWindow(Window *window)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    sim::addLog(sim_verbosity_debug, "window=%x", window);

    if(!window) return;

    window->hide();
}

void UI::onSetPosition(Window *window, int x, int y)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    if(!window) return;

    window->move(x, y);
}

void UI::onSetSize(Window *window, int w, int h)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    if(!window) return;

    window->resize(w, h);
}

void UI::onSetTitle(Window *window, std::string title)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    if(!window) return;

    window->setTitle(title);
}

void UI::onSetWindowEnabled(Window *window, bool enabled)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    if(!window) return;

    window->setEnabled(enabled);
}

#if WIDGET_IMAGE
void UI::onSetImage(Image *image, const char *data, int w, int h, bool resize)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    sim::addLog(sim_verbosity_debug, "image=%x, data=%x, w=%d, h=%d", image, data, w, h);

    image->setImage(data, w, h, resize);
}
#endif

void UI::onSceneChange(Window *window, int oldSceneID, int newSceneID)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    sim::addLog(sim_verbosity_debug, "window=%x, oldSceneID=%d, newSceneID=%d", window, oldSceneID, newSceneID);

    if(!window) return;

    window->onSceneChange(oldSceneID, newSceneID);
}

void UI::onSetEnabled(Widget *widget, bool enabled)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    sim::addLog(sim_verbosity_debug, "widget=%x, enabled=%d", widget, enabled);

    if(!widget) return;

    widget->getQWidget()->setEnabled(enabled);
}

#if WIDGET_EDIT
void UI::onSetEditValue(Edit *edit, std::string value, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    edit->setValue(value, suppressSignals);
}
#endif

#if WIDGET_SPINBOX
void UI::onSetSpinboxValue(Spinbox *spinbox, double value, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    spinbox->setValue(value, suppressSignals);
}
#endif

#if WIDGET_LABEL
void UI::onSetLabelText(Label *label, std::string text, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    label->setText(text, suppressSignals);
}
#endif

#if WIDGET_HSLIDER || WIDGET_VSLIDER
void UI::onSetSliderValue(Slider *slider, int value, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    slider->setValue(value, suppressSignals);
}
#endif

#if WIDGET_CHECKBOX
void UI::onSetCheckboxValue(Checkbox *checkbox, Qt::CheckState value, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    checkbox->setValue(value, suppressSignals);
}
#endif

#if WIDGET_RADIOBUTTON
void UI::onSetRadiobuttonValue(Radiobutton *radiobutton, bool value, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    radiobutton->setValue(value, suppressSignals);
}
#endif

#if WIDGET_COMBOBOX
void UI::onInsertComboboxItem(Combobox *combobox, int index, std::string text, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    combobox->insertItem(index, text, suppressSignals);
}

void UI::onRemoveComboboxItem(Combobox *combobox, int index, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    combobox->removeItem(index, suppressSignals);
}

void UI::onSetComboboxItems(Combobox *combobox, std::vector<std::string> items, int index, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    combobox->setItems(items, index, suppressSignals);
}

void UI::onSetComboboxSelectedIndex(Combobox *combobox, int index, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    combobox->setSelectedIndex(index, suppressSignals);
}
#endif

#if WIDGET_TABS
void UI::onSetCurrentTab(Tabs *tabs, int index, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    tabs->setCurrentTab(index, suppressSignals);
}
#endif

void UI::onSetWidgetVisibility(Widget *widget, bool visible)
{
    ASSERT_THREAD(UI);
    TRACE_FUNC;

    sim::addLog(sim_verbosity_debug, "widget=%x, visible=%d", widget, visible);

    if(!widget) return;

    widget->getQWidget()->setVisible(visible);
}

#if WIDGET_PLOT
void UI::onReplot(Plot *plot)
{
    plot->replot(true);
}

void UI::onAddCurve(Plot *plot, int type, std::string name, std::vector<int> color, int style, curve_options *opts)
{
    plot->addCurve(type, name, color, style, opts);
}

void UI::onAddCurveTimePoints(Plot *plot, std::string name, std::vector<double> x, std::vector<double> y)
{
    plot->addTimeData(name, x, y);
}

void UI::onAddCurveXYPoints(Plot *plot, std::string name, std::vector<double> t, std::vector<double> x, std::vector<double> y)
{
    plot->addXYData(name, t, x, y);
}

void UI::onClearCurve(Plot *plot, std::string name)
{
    plot->clearCurve(name);
}

void UI::onRemoveCurve(Plot *plot, std::string name)
{
    plot->removeCurve(name);
}

void UI::onSetPlotRanges(Plot *plot, double xmin, double xmax, double ymin, double ymax)
{
    QCustomPlot *qplot = static_cast<QCustomPlot*>(plot->getQWidget());
    qplot->xAxis->setRange(xmin, xmax);
    qplot->yAxis->setRange(ymin, ymax);
}

void UI::onSetPlotXRange(Plot *plot, double xmin, double xmax)
{
    QCustomPlot *qplot = static_cast<QCustomPlot*>(plot->getQWidget());
    qplot->xAxis->setRange(xmin, xmax);
}

void UI::onSetPlotYRange(Plot *plot, double ymin, double ymax)
{
    QCustomPlot *qplot = static_cast<QCustomPlot*>(plot->getQWidget());
    qplot->yAxis->setRange(ymin, ymax);
}

void UI::onGrowPlotRanges(Plot *plot, double xmin, double xmax, double ymin, double ymax)
{
    QCustomPlot *qplot = static_cast<QCustomPlot*>(plot->getQWidget());
    QCPRange xrange = qplot->xAxis->range();
    qplot->xAxis->setRange(xrange.lower - xmin, xrange.upper + xmax);
    QCPRange yrange = qplot->yAxis->range();
    qplot->yAxis->setRange(yrange.lower - ymin, yrange.upper + ymax);
}

void UI::onGrowPlotXRange(Plot *plot, double xmin, double xmax)
{
    QCustomPlot *qplot = static_cast<QCustomPlot*>(plot->getQWidget());
    QCPRange xrange = qplot->xAxis->range();
    qplot->xAxis->setRange(xrange.lower - xmin, xrange.upper + xmax);
}

void UI::onGrowPlotYRange(Plot *plot, double ymin, double ymax)
{
    QCustomPlot *qplot = static_cast<QCustomPlot*>(plot->getQWidget());
    QCPRange yrange = qplot->yAxis->range();
    qplot->yAxis->setRange(yrange.lower - ymin, yrange.upper + ymax);
}

void UI::onSetPlotLabels(Plot *plot, std::string x, std::string y)
{
    QCustomPlot *qplot = static_cast<QCustomPlot*>(plot->getQWidget());
    qplot->xAxis->setLabel(QString::fromStdString(x));
    qplot->yAxis->setLabel(QString::fromStdString(y));
}

void UI::onSetPlotXLabel(Plot *plot, std::string label)
{
    QCustomPlot *qplot = static_cast<QCustomPlot*>(plot->getQWidget());
    qplot->xAxis->setLabel(QString::fromStdString(label));
}

void UI::onSetPlotYLabel(Plot *plot, std::string label)
{
    QCustomPlot *qplot = static_cast<QCustomPlot*>(plot->getQWidget());
    qplot->yAxis->setLabel(QString::fromStdString(label));
}

void UI::onRescaleAxes(Plot *plot, std::string name, bool onlyEnlargeX, bool onlyEnlargeY)
{
    plot->rescaleAxes(name, onlyEnlargeX, onlyEnlargeY);
}

void UI::onRescaleAxesAll(Plot *plot, bool onlyEnlargeX, bool onlyEnlargeY)
{
    plot->rescaleAxesAll(onlyEnlargeX, onlyEnlargeY);
}

void UI::onSetMouseOptions(Plot *plot, bool panX, bool panY, bool zoomX, bool zoomY)
{
    plot->setMouseOptions(panX, panY, zoomX, zoomY);
}

void UI::onSetLegendVisibility(Plot *plot, bool visible)
{
    plot->setLegendVisibility(visible);
}
#endif

#if WIDGET_TABLE
void UI::onClearTable(Table *table, bool suppressSignals)
{
    table->clear(suppressSignals);
}

void UI::onSetRowCount(Table *table, int count, bool suppressSignals)
{
    table->setRowCount(count, suppressSignals);
}

void UI::onSetColumnCountTable(Table *table, int count, bool suppressSignals)
{
    table->setColumnCount(count, suppressSignals);
}

void UI::onSetItem(Table *table, int row, int column, std::string text, bool suppressSignals)
{
    table->setItem(row, column, text, suppressSignals);
}

void UI::onSetItemImage(Table *table, int row, int column, std::string data, int width, int height, bool suppressSignals)
{
    table->setItemImage(row, column, data, width, height, suppressSignals);
}

void UI::onSetRowHeaderText(Table *table, int row, std::string text)
{
    table->setRowHeaderText(row, text);
}

void UI::onSetColumnHeaderTextTable(Table *table, int column, std::string text)
{
    table->setColumnHeaderText(column, text);
}

void UI::onSetItemEditable(Table *table, int row, int column, bool editable)
{
    table->setItemEditable(row, column, editable);
}

void UI::onRestoreStateTable(Table *table, std::string state)
{
    table->restoreState(state);
}

void UI::onSetRowHeight(Table *table, int row, int min_size, int max_size)
{
    table->setRowHeight(row, min_size, max_size);
}

void UI::onSetColumnWidthTable(Table *table, int column, int min_size, int max_size)
{
    table->setColumnWidth(column, min_size, max_size);
}

void UI::onSetTableSelection(Table *table, int row, int column, bool suppressSignals)
{
    table->setSelection(row, column, suppressSignals);
}
#endif

#if WIDGET_PROGRESSBAR
void UI::onSetProgress(Progressbar *progressbar, int value)
{
    progressbar->setValue(value);
}
#endif

#if WIDGET_TREE
void UI::onSetColumnCountTree(Tree *tree, int count, bool suppressSignals)
{
    tree->setColumnCount(count, suppressSignals);
}

void UI::onSetColumnHeaderTextTree(Tree *tree, int column, std::string text)
{
    tree->setColumnHeaderText(column, text);
}

void UI::onRestoreStateTree(Tree *tree, std::string state)
{
    tree->restoreState(state);
}

void UI::onSetColumnWidthTree(Tree *tree, int column, int min_size, int max_size)
{
    tree->setColumnWidth(column, min_size, max_size);
}

void UI::onClearTree(Tree *tree, bool suppressSignals)
{
    tree->clear(suppressSignals);
}

void UI::onAddTreeItem(Tree *tree, int item_id, int parent_id, std::vector<std::string> text, bool expanded, bool suppressSignals)
{
    tree->addItem(item_id, parent_id, text, expanded, suppressSignals);
}

void UI::onUpdateTreeItemText(Tree *tree, int item_id, std::vector<std::string> text)
{
    tree->updateItemText(item_id, text);
}

void UI::onUpdateTreeItemParent(Tree *tree, int item_id, int parent_id, bool suppressSignals)
{
    tree->updateItemParent(item_id, parent_id, suppressSignals);
}

void UI::onRemoveTreeItem(Tree *tree, int item_id, bool suppressSignals)
{
    tree->removeItem(item_id, suppressSignals);
}

void UI::onSetTreeSelection(Tree *tree, int item_id, bool suppressSignals)
{
    tree->setSelection(item_id, suppressSignals);
}

void UI::onExpandAll(Tree *tree, bool suppressSignals)
{
    tree->expandAll(suppressSignals);
}

void UI::onCollapseAll(Tree *tree, bool suppressSignals)
{
    tree->collapseAll(suppressSignals);
}

void UI::onExpandToDepth(Tree *tree, int depth, bool suppressSignals)
{
    tree->expandToDepth(depth, suppressSignals);
}
#endif

#if WIDGET_TEXTBROWSER
void UI::onSetText(TextBrowser *textbrowser, std::string text, bool suppressSignals)
{
    textbrowser->setText(text, suppressSignals);
}

void UI::onSetUrl(TextBrowser *textbrowser, std::string url)
{
    textbrowser->setUrl(url);
}
#endif

#if WIDGET_SCENE3D
void UI::onAddScene3DNode(Scene3D *scene3d, int id, int parentId, int type)
{
    scene3d->addNode(id, parentId, type);
}

void UI::onRemoveScene3DNode(Scene3D *scene3d, int id)
{
    scene3d->removeNode(id);
}

void UI::onSetScene3DNodeEnabled(Scene3D *scene3d, int id, bool enabled)
{
    scene3d->enableNode(id, enabled);
}

void UI::onSetScene3DIntParam(Scene3D *scene3d, int id, std::string param, int value)
{
    scene3d->setIntParameter(id, param, value);
}

void UI::onSetScene3DFloatParam(Scene3D *scene3d, int id, std::string param, double value)
{
    scene3d->setFloatParameter(id, param, value);
}

void UI::onSetScene3DStringParam(Scene3D *scene3d, int id, std::string param, std::string value)
{
    scene3d->setStringParameter(id, param, value);
}

void UI::onSetScene3DVector2Param(Scene3D *scene3d, int id, std::string param, double x, double y)
{
    scene3d->setVector2Parameter(id, param, x, y);
}

void UI::onSetScene3DVector3Param(Scene3D *scene3d, int id, std::string param, double x, double y, double z)
{
    scene3d->setVector3Parameter(id, param, x, y, z);
}

void UI::onSetScene3DVector4Param(Scene3D *scene3d, int id, std::string param, double x, double y, double z, double w)
{
    scene3d->setVector4Parameter(id, param, x, y, z, w);
}
#endif

#if WIDGET_SVG
void UI::onSvgLoadFile(SVG *svg, const QString &file)
{
    svg->loadFile(file);
}

void UI::onSvgLoadData(SVG *svg, const QByteArray &data)
{
    svg->loadData(data);
}
#endif

