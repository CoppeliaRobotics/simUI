#include "UIProxy.h"
#include "debug.h"

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

#include "stubs.h"

using namespace tinyxml2;

// UIProxy is a singleton

UIProxy *UIProxy::instance = NULL;

QWidget *UIProxy::simMainWindow = NULL;

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
        UIProxy::simMainWindow = (QWidget *)simGetMainWindow(1);
        simGetFloatParameter(sim_floatparam_mouse_wheel_zoom_factor, &UIProxy::wheelZoomFactor);

        uiThread(); // we remember this currentThreadId as the "UI" thread

        DEBUG_OUT << "UIProxy(" << UIProxy::instance << ") constructed in thread " << QThread::currentThreadId() << std::endl;
    }
    return UIProxy::instance;
}

void UIProxy::destroyInstance()
{
    DEBUG_OUT << "[enter]" << std::endl;

    if(UIProxy::instance)
    {
        delete UIProxy::instance;

        DEBUG_OUT << "destroyed UIProxy instance" << std::endl;
    }

    DEBUG_OUT << "[leave]" << std::endl;
}

void UIProxy::onMsgBox(int type, int buttons, std::string title, std::string message, int *result)
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
    case sim_ui_msgbox_type_info:
        msgBox.setIcon(QMessageBox::Information);
        break;
    case sim_ui_msgbox_type_question:
        msgBox.setIcon(QMessageBox::Question);
        break;
    case sim_ui_msgbox_type_warning:
        msgBox.setIcon(QMessageBox::Warning);
        break;
    case sim_ui_msgbox_type_critical:
        msgBox.setIcon(QMessageBox::Critical);
        break;
    }
    switch(buttons)
    {
    case sim_ui_msgbox_buttons_ok:
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        break;
    case sim_ui_msgbox_buttons_yesno:
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        break;
    case sim_ui_msgbox_buttons_yesnocancel:
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        break;
    case sim_ui_msgbox_buttons_okcancel:
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        break;
    }
    int ret = msgBox.exec();
    switch(ret)
    {
    case QMessageBox::Ok:
        *result = sim_ui_msgbox_result_ok;
        break;
    case QMessageBox::Cancel:
        *result = sim_ui_msgbox_result_cancel;
        break;
    case QMessageBox::Yes:
        *result = sim_ui_msgbox_result_yes;
        break;
    case QMessageBox::No:
        *result = sim_ui_msgbox_result_no;
        break;
    }
}

void UIProxy::onFileDialog(int type, std::string title, std::string startPath, std::string initName, std::string extName, std::string ext, bool native, std::vector<std::string> *result)
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
    case sim_ui_filedialog_type_load:
        file = QFileDialog::getOpenFileName(nullptr, qtitle, qstartPath, filter, nullptr, opts);
        result->push_back(file.toStdString());
        break;
    case sim_ui_filedialog_type_load_multiple:
        files = QFileDialog::getOpenFileNames(nullptr, qtitle, qstartPath, filter, nullptr, opts);
        for(auto file : files)
            result->push_back(file.toStdString());
        break;
    case sim_ui_filedialog_type_save:
        file = QFileDialog::getSaveFileName(nullptr, qtitle, qstartPath, filter, nullptr, opts);
        result->push_back(file.toStdString());
        break;
    case sim_ui_filedialog_type_folder:
        file = QFileDialog::getExistingDirectory(nullptr, qtitle, qstartPath, opts | QFileDialog::ShowDirsOnly);
        result->push_back(file.toStdString());
        break;
    }
}

void UIProxy::onCreate(Proxy *proxy)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    proxy->createQtWidget(this);

    DEBUG_OUT << "[leave]" << std::endl;
}

// The following slots are directly connected to Qt Widgets' signals.
// Here we look up the sender to find the corresponding Widget object,
// and emit a signal with the Widget object pointer.
//
// That signal will be connected to a slot in UIFunctions, such
// that the callback is called from the SIM thread.

#if WIDGET_BUTTON || WIDGET_RADIOBUTTON
void UIProxy::onButtonClick()
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    QWidget *qwidget = dynamic_cast<QWidget*>(sender());
    if(qwidget)
    {
        Widget *widget = Widget::byQWidget(qwidget);
        if(widget)
        {
            emit buttonClick(widget);
        }
    }

    DEBUG_OUT << "[leave]" << std::endl;
}
#endif

#if WIDGET_LABEL
void UIProxy::onLinkActivated(const QString &link)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    QWidget *qwidget = dynamic_cast<QWidget*>(sender());
    if(qwidget)
    {
        Widget *widget = Widget::byQWidget(qwidget);
        if(widget)
        {
            emit linkActivated(widget, link);
        }
    }

    DEBUG_OUT << "[leave]" << std::endl;
}
#endif

void UIProxy::onValueChangeInt(int value)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    QWidget *qwidget = dynamic_cast<QWidget*>(sender());
    if(qwidget)
    {
        Widget *widget = Widget::byQWidget(qwidget);
        if(widget)
        {
            emit valueChangeInt(widget, value);
        }
    }

    DEBUG_OUT << "[leave]" << std::endl;
}

void UIProxy::onValueChangeDouble(double value)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    QWidget *qwidget = dynamic_cast<QWidget*>(sender());
    if(qwidget)
    {
        Widget *widget = Widget::byQWidget(qwidget);
        if(widget)
        {
            emit valueChangeDouble(widget, value);
        }
    }

    DEBUG_OUT << "[leave]" << std::endl;
}

void UIProxy::onValueChangeString(QString value)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    QWidget *qwidget = dynamic_cast<QWidget*>(sender());
    if(qwidget)
    {
        Widget *widget = Widget::byQWidget(qwidget);
        if(widget)
        {
            emit valueChangeString(widget, value);
        }
    }

    DEBUG_OUT << "[leave]" << std::endl;
}

#if WIDGET_EDIT
void UIProxy::onEditingFinished()
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

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

    DEBUG_OUT << "[leave]" << std::endl;
}
#endif

#if WIDGET_PLOT
void UIProxy::onPlottableClick(QCPAbstractPlottable *plottable, int index, QMouseEvent *event)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

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

    DEBUG_OUT << "[leave]" << std::endl;
}

void UIProxy::onLegendClick(QCPLegend *legend, QCPAbstractLegendItem *item, QMouseEvent *event)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

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

    DEBUG_OUT << "[leave]" << std::endl;
}
#endif

#if WIDGET_TABLE
void UIProxy::onCellActivate(int row, int col)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

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

    DEBUG_OUT << "[leave]" << std::endl;
}

void UIProxy::onTableSelectionChange()
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

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

    DEBUG_OUT << "[leave]" << std::endl;
}
#endif

#if WIDGET_TREE
void UIProxy::onTreeSelectionChange()
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

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

    DEBUG_OUT << "[leave]" << std::endl;
}
#endif

#if WIDGET_IMAGE
void UIProxy::onMouseEvent(Image *image, int type, bool shift, bool control, int x, int y)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    emit mouseEvent(image, type, shift, control, x, y);

    DEBUG_OUT << "[leave]" << std::endl;
}
#endif

#if WIDGET_DATAFLOW
void UIProxy::onNodeAdded(QDataflowModelNode *node)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    QDataflowModel *model = dynamic_cast<QDataflowModel*>(sender());

    if(model)
    {
        Dataflow *dataflow = Dataflow::byModel(model);
        if(dataflow)
        {
            int id = dataflow->nextId();
            dataflow->mapNode(node, id);
            emit nodeAdded(dataflow, id, node->pos(), node->text(), node->inletCount(), node->outletCount());
        }
    }

    DEBUG_OUT << "[leave]" << std::endl;
}

void UIProxy::onNodeRemoved(QDataflowModelNode *node)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    QDataflowModel *model = dynamic_cast<QDataflowModel*>(sender());

    if(model)
    {
        Dataflow *dataflow = Dataflow::byModel(model);
        if(dataflow)
        {
            int id = dataflow->getNodeId(node);
            dataflow->unmapNode(node);
            emit nodeRemoved(dataflow, id);
        }
    }

    DEBUG_OUT << "[leave]" << std::endl;
}

void UIProxy::onNodeValidChanged(QDataflowModelNode *node, bool valid)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    QDataflowModel *model = dynamic_cast<QDataflowModel*>(sender());

    if(model)
    {
        Dataflow *dataflow = Dataflow::byModel(model);
        if(dataflow)
        {
            int id = dataflow->getNodeId(node);
            emit nodeValidChanged(dataflow, id, valid);
        }
    }

    DEBUG_OUT << "[leave]" << std::endl;
}

void UIProxy::onNodePosChanged(QDataflowModelNode *node, QPoint pos)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    QDataflowModel *model = dynamic_cast<QDataflowModel*>(sender());

    if(model)
    {
        Dataflow *dataflow = Dataflow::byModel(model);
        if(dataflow)
        {
            int id = dataflow->getNodeId(node);
            emit nodePosChanged(dataflow, id, pos);
        }
    }

    DEBUG_OUT << "[leave]" << std::endl;
}

void UIProxy::onNodeTextChanged(QDataflowModelNode *node, QString text)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    QDataflowModel *model = dynamic_cast<QDataflowModel*>(sender());

    if(model)
    {
        Dataflow *dataflow = Dataflow::byModel(model);
        if(dataflow)
        {
            int id = dataflow->getNodeId(node);
            emit nodeTextChanged(dataflow, id, text);
        }
    }

    DEBUG_OUT << "[leave]" << std::endl;
}

void UIProxy::onNodeInletCountChanged(QDataflowModelNode *node, int count)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    QDataflowModel *model = dynamic_cast<QDataflowModel*>(sender());

    if(model)
    {
        Dataflow *dataflow = Dataflow::byModel(model);
        if(dataflow)
        {
            int id = dataflow->getNodeId(node);
            emit nodeInletCountChanged(dataflow, id, count);
        }
    }

    DEBUG_OUT << "[leave]" << std::endl;
}

void UIProxy::onNodeOutletCountChanged(QDataflowModelNode *node, int count)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    QDataflowModel *model = dynamic_cast<QDataflowModel*>(sender());

    if(model)
    {
        Dataflow *dataflow = Dataflow::byModel(model);
        if(dataflow)
        {
            int id = dataflow->getNodeId(node);
            emit nodeOutletCountChanged(dataflow, id, count);
        }
    }

    DEBUG_OUT << "[leave]" << std::endl;
}

void UIProxy::onConnectionAdded(QDataflowModelConnection *conn)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    QDataflowModel *model = dynamic_cast<QDataflowModel*>(sender());

    if(model)
    {
        Dataflow *dataflow = Dataflow::byModel(model);
        if(dataflow)
        {
            int srcid = dataflow->getNodeId(conn->source()->node());
            int dstid = dataflow->getNodeId(conn->source()->node());
            emit connectionAdded(dataflow, srcid, conn->source()->index(), dstid, conn->dest()->index());
        }
    }

    DEBUG_OUT << "[leave]" << std::endl;
}

void UIProxy::onConnectionRemoved(QDataflowModelConnection *conn)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    QDataflowModel *model = dynamic_cast<QDataflowModel*>(sender());

    if(model)
    {
        Dataflow *dataflow = Dataflow::byModel(model);
        if(dataflow)
        {
            int srcid = dataflow->getNodeId(conn->source()->node());
            int dstid = dataflow->getNodeId(conn->source()->node());
            emit connectionRemoved(dataflow, srcid, conn->source()->index(), dstid, conn->dest()->index());
        }
    }

    DEBUG_OUT << "[leave]" << std::endl;
}
#endif

#if WIDGET_TEXTBROWSER
void UIProxy::onTextChanged()
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    if(QTextBrowser *qtextbrowser = dynamic_cast<QTextBrowser*>(sender()))
    {
        if(TextBrowser *textbrowser = dynamic_cast<TextBrowser*>(Widget::byQWidget(qtextbrowser)))
        {
            std::string text = textbrowser->getText();
            emit valueChangeString(textbrowser, QString::fromStdString(text));
        }
    }

    DEBUG_OUT << "[leave]" << std::endl;
}

void UIProxy::onAnchorClicked(const QUrl &link)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    if(QTextBrowser *qtextbrowser = dynamic_cast<QTextBrowser*>(sender()))
    {
        if(TextBrowser *textbrowser = dynamic_cast<TextBrowser*>(Widget::byQWidget(qtextbrowser)))
        {
            emit linkActivated(textbrowser, link.url());
        }
    }

    DEBUG_OUT << "[leave]" << std::endl;
}
#endif

#if WIDGET_SCENE3D
void UIProxy::onViewCenterChanged(const QVector3D &viewCenter)
{
    std::cout << "View center changed: " << viewCenter.x() << ", " << viewCenter.y() << ", " << viewCenter.z() << std::endl;
}

void UIProxy::onPositionChanged(const QVector3D &position)
{
    std::cout << "Position changed: " << position.x() << ", " << position.y() << ", " << position.z() << std::endl;
}

void UIProxy::onScene3DObjectClicked(Qt3DRender::QPickEvent *pick)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

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
            DEBUG_OUT << "received QPickEvent but sender() is not associated with a Scene3D widget" << std::endl;
        }
    }
    else
    {
        DEBUG_OUT << "received QPickEvent but sender() is not a QObjectPicker" << std::endl;
    }

    DEBUG_OUT << "[leave]" << std::endl;
}
#endif

// The following slots are wrappers for functions called from SIM thread
// which should instead execute in the UI thread.

void UIProxy::onDestroy(Proxy *proxy)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    DEBUG_OUT << "proxy=" << (void*)proxy << std::endl;

    if(!proxy)
    {
        DEBUG_OUT << "WARNING: proxy is NULL" << std::endl;
        return;
    }
    if(!proxy->ui)
    {
        DEBUG_OUT << "WARNING: proxy->ui is NULL" << std::endl;
        return;
    }
    if(!proxy->ui->qwidget)
    {
        DEBUG_OUT << "WARNING: proxy->ui->qwidget is NULL" << std::endl;
        return;
    }

    DEBUG_OUT << "calling proxy->ui->qwidget->deleteLater()..." << std::endl;
    proxy->ui->qwidget->deleteLater();

    DEBUG_OUT << "[leave]" << std::endl;
}

void UIProxy::onSetStyleSheet(Widget *widget, std::string styleSheet)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    widget->getQWidget()->setStyleSheet(QString::fromStdString(styleSheet));

    DEBUG_OUT << "[leave]" << std::endl;
}

#if WIDGET_BUTTON
void UIProxy::onSetButtonText(Button *button, std::string text)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    button->setText(text);

    DEBUG_OUT << "[leave]" << std::endl;
}

void UIProxy::onSetButtonPressed(Button *button, bool pressed)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    button->setPressed(pressed);

    DEBUG_OUT << "[leave]" << std::endl;
}
#endif

void UIProxy::onShowWindow(Window *window)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    DEBUG_OUT << "window=" << (void*)window << std::endl;

    if(!window) return;

    window->show();

    DEBUG_OUT << "[leave]" << std::endl;
}

void UIProxy::onHideWindow(Window *window)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    DEBUG_OUT << "window=" << (void*)window << std::endl;

    if(!window) return;

    window->hide();

    DEBUG_OUT << "[leave]" << std::endl;
}

void UIProxy::onSetPosition(Window *window, int x, int y)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    if(!window) return;

    window->move(x, y);

    DEBUG_OUT << "[leave]" << std::endl;
}

void UIProxy::onSetSize(Window *window, int w, int h)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    if(!window) return;

    window->resize(w, h);

    DEBUG_OUT << "[leave]" << std::endl;
}

void UIProxy::onSetTitle(Window *window, std::string title)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    if(!window) return;

    window->setTitle(title);

    DEBUG_OUT << "[leave]" << std::endl;
}

#if WIDGET_IMAGE
void UIProxy::onSetImage(Image *image, const char *data, int w, int h)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    DEBUG_OUT << "image=" << (void*)image << ", data=" << (void*)data << ", w=" << w << ", h=" << h << std::endl;

    image->setImage(data, w, h);

    DEBUG_OUT << "[leave]" << std::endl;
}
#endif

void UIProxy::onSceneChange(Window *window, int oldSceneID, int newSceneID)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    DEBUG_OUT << "window=" << (void*)window << ", oldSceneID=" << oldSceneID << ", newSceneID" << newSceneID << std::endl;

    if(!window) return;

    window->onSceneChange(oldSceneID, newSceneID);

    DEBUG_OUT << "[leave]" << std::endl;
}

void UIProxy::onSetEnabled(Widget *widget, bool enabled)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    DEBUG_OUT << "widget=" << (void*)widget << ", enabled=" << enabled << std::endl;

    if(!widget) return;

    widget->getQWidget()->setEnabled(enabled);

    DEBUG_OUT << "[leave]" << std::endl;
}

#if WIDGET_EDIT
void UIProxy::onSetEditValue(Edit *edit, std::string value, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    edit->setValue(value, suppressSignals);

    DEBUG_OUT << "[leave]" << std::endl;
}
#endif

#if WIDGET_SPINBOX
void UIProxy::onSetSpinboxValue(Spinbox *spinbox, double value, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    spinbox->setValue(value, suppressSignals);

    DEBUG_OUT << "[leave]" << std::endl;
}
#endif

#if WIDGET_LABEL
void UIProxy::onSetLabelText(Label *label, std::string text, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    label->setText(text, suppressSignals);

    DEBUG_OUT << "[leave]" << std::endl;
}
#endif

#if WIDGET_HSLIDER || WIDGET_VSLIDER
void UIProxy::onSetSliderValue(Slider *slider, int value, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    slider->setValue(value, suppressSignals);

    DEBUG_OUT << "[leave]" << std::endl;
}
#endif

#if WIDGET_CHECKBOX
void UIProxy::onSetCheckboxValue(Checkbox *checkbox, Qt::CheckState value, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    checkbox->setValue(value, suppressSignals);

    DEBUG_OUT << "[leave]" << std::endl;
}
#endif

#if WIDGET_RADIOBUTTON
void UIProxy::onSetRadiobuttonValue(Radiobutton *radiobutton, bool value, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    radiobutton->setValue(value, suppressSignals);

    DEBUG_OUT << "[leave]" << std::endl;
}
#endif

#if WIDGET_COMBOBOX
void UIProxy::onInsertComboboxItem(Combobox *combobox, int index, std::string text, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    combobox->insertItem(index, text, suppressSignals);

    DEBUG_OUT << "[leave]" << std::endl;
}

void UIProxy::onRemoveComboboxItem(Combobox *combobox, int index, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    combobox->removeItem(index, suppressSignals);

    DEBUG_OUT << "[leave]" << std::endl;
}

void UIProxy::onSetComboboxItems(Combobox *combobox, std::vector<std::string> items, int index, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    combobox->setItems(items, index, suppressSignals);

    DEBUG_OUT << "[leave]" << std::endl;
}

void UIProxy::onSetComboboxSelectedIndex(Combobox *combobox, int index, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    combobox->setSelectedIndex(index, suppressSignals);

    DEBUG_OUT << "[leave]" << std::endl;
}
#endif

#if WIDGET_TABS
void UIProxy::onSetCurrentTab(Tabs *tabs, int index, bool suppressSignals)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    tabs->setCurrentTab(index, suppressSignals);

    DEBUG_OUT << "[leave]" << std::endl;
}
#endif

void UIProxy::onSetWidgetVisibility(Widget *widget, bool visible)
{
    ASSERT_THREAD(UI);
    DEBUG_OUT << "[enter]" << std::endl;

    DEBUG_OUT << "widget=" << (void*)widget << ", visible=" << visible << std::endl;

    if(!widget) return;

    widget->getQWidget()->setVisible(visible);

    DEBUG_OUT << "[leave]" << std::endl;
}

#if WIDGET_PLOT
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
#endif

#if WIDGET_TABLE
void UIProxy::onClearTable(Table *table, bool suppressSignals)
{
    table->clear(suppressSignals);
}

void UIProxy::onSetRowCount(Table *table, int count, bool suppressSignals)
{
    table->setRowCount(count, suppressSignals);
}

void UIProxy::onSetColumnCountTable(Table *table, int count, bool suppressSignals)
{
    table->setColumnCount(count, suppressSignals);
}

void UIProxy::onSetItem(Table *table, int row, int column, std::string text, bool suppressSignals)
{
    table->setItem(row, column, text, suppressSignals);
}

void UIProxy::onSetItemImage(Table *table, int row, int column, std::string data, int width, int height, bool suppressSignals)
{
    table->setItemImage(row, column, data, width, height, suppressSignals);
}

void UIProxy::onSetRowHeaderText(Table *table, int row, std::string text)
{
    table->setRowHeaderText(row, text);
}

void UIProxy::onSetColumnHeaderTextTable(Table *table, int column, std::string text)
{
    table->setColumnHeaderText(column, text);
}

void UIProxy::onSetItemEditable(Table *table, int row, int column, bool editable)
{
    table->setItemEditable(row, column, editable);
}

void UIProxy::onRestoreStateTable(Table *table, std::string state)
{
    table->restoreState(state);
}

void UIProxy::onSetRowHeight(Table *table, int row, int min_size, int max_size)
{
    table->setRowHeight(row, min_size, max_size);
}

void UIProxy::onSetColumnWidthTable(Table *table, int column, int min_size, int max_size)
{
    table->setColumnWidth(column, min_size, max_size);
}

void UIProxy::onSetTableSelection(Table *table, int row, int column, bool suppressSignals)
{
    table->setSelection(row, column, suppressSignals);
}
#endif

#if WIDGET_PROGRESSBAR
void UIProxy::onSetProgress(Progressbar *progressbar, int value)
{
    progressbar->setValue(value);
}
#endif

#if WIDGET_TREE
void UIProxy::onSetColumnCountTree(Tree *tree, int count, bool suppressSignals)
{
    tree->setColumnCount(count, suppressSignals);
}

void UIProxy::onSetColumnHeaderTextTree(Tree *tree, int column, std::string text)
{
    tree->setColumnHeaderText(column, text);
}

void UIProxy::onRestoreStateTree(Tree *tree, std::string state)
{
    tree->restoreState(state);
}

void UIProxy::onSetColumnWidthTree(Tree *tree, int column, int min_size, int max_size)
{
    tree->setColumnWidth(column, min_size, max_size);
}

void UIProxy::onClearTree(Tree *tree, bool suppressSignals)
{
    tree->clear(suppressSignals);
}

void UIProxy::onAddTreeItem(Tree *tree, int item_id, int parent_id, std::vector<std::string> text, bool expanded, bool suppressSignals)
{
    tree->addItem(item_id, parent_id, text, expanded, suppressSignals);
}

void UIProxy::onUpdateTreeItemText(Tree *tree, int item_id, std::vector<std::string> text)
{
    tree->updateItemText(item_id, text);
}

void UIProxy::onUpdateTreeItemParent(Tree *tree, int item_id, int parent_id, bool suppressSignals)
{
    tree->updateItemParent(item_id, parent_id, suppressSignals);
}

void UIProxy::onRemoveTreeItem(Tree *tree, int item_id, bool suppressSignals)
{
    tree->removeItem(item_id, suppressSignals);
}

void UIProxy::onSetTreeSelection(Tree *tree, int item_id, bool suppressSignals)
{
    tree->setSelection(item_id, suppressSignals);
}

void UIProxy::onExpandAll(Tree *tree, bool suppressSignals)
{
    tree->expandAll(suppressSignals);
}

void UIProxy::onCollapseAll(Tree *tree, bool suppressSignals)
{
    tree->collapseAll(suppressSignals);
}

void UIProxy::onExpandToDepth(Tree *tree, int depth, bool suppressSignals)
{
    tree->expandToDepth(depth, suppressSignals);
}
#endif

#if WIDGET_DATAFLOW
void UIProxy::onAddNode(Dataflow *dataflow, int id, QPoint pos, QString text, int inlets, int outlets)
{
    dataflow->addNode(id, pos, text, inlets, outlets);
}

void UIProxy::onRemoveNode(Dataflow *dataflow, int id)
{
    dataflow->removeNode(id);
}

void UIProxy::onSetNodeValid(Dataflow *dataflow, int id, bool valid)
{
    dataflow->setNodeValid(id, valid);
}

void UIProxy::onSetNodePos(Dataflow *dataflow, int id, QPoint pos)
{
    dataflow->setNodePos(id, pos);
}

void UIProxy::onSetNodeText(Dataflow *dataflow, int id, QString text)
{
    dataflow->setNodeText(id, text);
}

void UIProxy::onSetNodeInletCount(Dataflow *dataflow, int id, int count)
{
    dataflow->setNodeInletCount(id, count);
}

void UIProxy::onSetNodeOutletCount(Dataflow *dataflow, int id, int count)
{
    dataflow->setNodeOutletCount(id, count);
}

void UIProxy::onAddConnection(Dataflow *dataflow, int srcId, int srcOutlet, int dstId, int dstInlet)
{
    dataflow->addConnection(srcId, srcOutlet, dstId, dstInlet);
}

void UIProxy::onRemoveConnection(Dataflow *dataflow, int srcId, int srcOutlet, int dstId, int dstInlet)
{
    dataflow->removeConnection(srcId, srcOutlet, dstId, dstInlet);
}
#endif

#if WIDGET_TEXTBROWSER
void UIProxy::onSetText(TextBrowser *textbrowser, std::string text, bool suppressSignals)
{
    textbrowser->setText(text, suppressSignals);
}

void UIProxy::onSetUrl(TextBrowser *textbrowser, std::string url)
{
    textbrowser->setUrl(url);
}
#endif

#if WIDGET_SCENE3D
void UIProxy::onAddScene3DNode(Scene3D *scene3d, int id, int parentId, int type)
{
    scene3d->addNode(id, parentId, type);
}

void UIProxy::onRemoveScene3DNode(Scene3D *scene3d, int id)
{
    scene3d->removeNode(id);
}

void UIProxy::onSetScene3DNodeEnabled(Scene3D *scene3d, int id, bool enabled)
{
    scene3d->enableNode(id, enabled);
}

void UIProxy::onSetScene3DIntParam(Scene3D *scene3d, int id, std::string param, int value)
{
    scene3d->setIntParameter(id, param, value);
}

void UIProxy::onSetScene3DFloatParam(Scene3D *scene3d, int id, std::string param, float value)
{
    scene3d->setFloatParameter(id, param, value);
}

void UIProxy::onSetScene3DStringParam(Scene3D *scene3d, int id, std::string param, std::string value)
{
    scene3d->setStringParameter(id, param, value);
}

void UIProxy::onSetScene3DVector2Param(Scene3D *scene3d, int id, std::string param, float x, float y)
{
    scene3d->setVector2Parameter(id, param, x, y);
}

void UIProxy::onSetScene3DVector3Param(Scene3D *scene3d, int id, std::string param, float x, float y, float z)
{
    scene3d->setVector3Parameter(id, param, x, y, z);
}

void UIProxy::onSetScene3DVector4Param(Scene3D *scene3d, int id, std::string param, float x, float y, float z, float w)
{
    scene3d->setVector4Parameter(id, param, x, y, z, w);
}
#endif

