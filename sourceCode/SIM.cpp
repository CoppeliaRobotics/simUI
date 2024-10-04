#include "SIM.h"
#include "UI.h"

#include <QThread>

#include <iostream>

#include <simPlusPlus/Lib.h>
#include "stubs.h"

// SIM is a singleton

SIM *SIM::instance = nullptr;

SIM::SIM(QObject *parent)
    : QObject(parent)
{
}

SIM::~SIM()
{
    SIM::instance = nullptr;
}

SIM * SIM::getInstance(QObject *parent)
{
    if(!SIM::instance)
    {
        SIM::instance = new SIM(parent);

        simThread(); // we remember of this currentThreadId as the "SIM" thread

        sim::addLog(sim_verbosity_debug, "SIM(%x) constructed in thread %s", SIM::instance, QThread::currentThreadId());
    }
    return SIM::instance;
}

void SIM::destroyInstance()
{
    TRACE_FUNC;

    if(SIM::instance)
    {
        delete SIM::instance;

        sim::addLog(sim_verbosity_debug, "destroyed SIM instance");
    }
}

void SIM::connectSignals()
{
    UI *ui = UI::getInstance();
    // connect signals/slots from UI to SIM and vice-versa
    connect(this, &SIM::msgBox, ui, &UI::onMsgBox, Qt::BlockingQueuedConnection);
    connect(this, &SIM::fileDialog, ui, &UI::onFileDialog, Qt::BlockingQueuedConnection);
    connect(this, &SIM::colorDialog, ui, &UI::onColorDialog, Qt::BlockingQueuedConnection);
    connect(this, &SIM::inputDialog, ui, &UI::onInputDialog, Qt::BlockingQueuedConnection);
    connect(this, &SIM::create, ui, &UI::onCreate, Qt::BlockingQueuedConnection);
#if WIDGET_BUTTON
    connect(ui, &UI::buttonClick, this, &SIM::onButtonClick);
#endif
#if WIDGET_LABEL
    connect(ui, &UI::linkActivated, this, &SIM::onLinkActivated);
#endif
    connect(ui, &UI::valueChangeInt, this, &SIM::onValueChangeInt);
    connect(ui, &UI::valueChangeDouble, this, &SIM::onValueChangeDouble);
    connect(ui, &UI::valueChangeString, this, &SIM::onValueChangeString);
#if WIDGET_EDIT
    connect(ui, &UI::editingFinished, this, &SIM::onEditingFinished);
    connect(ui, &UI::evaluateExpressionInSandbox, this, &SIM::onEvaluateExpressionInSandbox);
#endif
    connect(ui, &UI::windowClose, this, &SIM::onWindowClose);
    connect(this, &SIM::destroy, ui, &UI::onDestroy, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setStyleSheet, ui, &UI::onSetStyleSheet, Qt::BlockingQueuedConnection);
#if WIDGET_BUTTON
    connect(this, &SIM::setButtonText, ui, &UI::onSetButtonText, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setButtonPressed, ui, &UI::onSetButtonPressed, Qt::BlockingQueuedConnection);
#endif
    connect(this, &SIM::showWindow, ui, &UI::onShowWindow, Qt::BlockingQueuedConnection);
    connect(this, &SIM::adjustSize, ui, &UI::onAdjustSize, Qt::BlockingQueuedConnection);
    connect(this, &SIM::hideWindow, ui, &UI::onHideWindow, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setPosition, ui, &UI::onSetPosition, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setSize, ui, &UI::onSetSize, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setTitle, ui, &UI::onSetTitle, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setWindowEnabled, ui, &UI::onSetWindowEnabled, Qt::BlockingQueuedConnection);
#if WIDGET_IMAGE
    connect(this, &SIM::setImage, ui, &UI::onSetImage, Qt::BlockingQueuedConnection);
    connect(ui, &UI::loadImageFromFile, this, &SIM::onLoadImageFromFile);
#endif
    connect(this, &SIM::sceneChange, ui, &UI::onSceneChange, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setEnabled, ui, &UI::onSetEnabled, Qt::BlockingQueuedConnection);
#if WIDGET_EDIT
    connect(this, &SIM::setEditValue, ui, &UI::onSetEditValue, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setEvaluationResult, ui, &UI::onSetEvaluationResult, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_SPINBOX
    connect(this, &SIM::setSpinboxValue, ui, &UI::onSetSpinboxValue, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_LABEL
    connect(this, &SIM::setLabelText, ui, &UI::onSetLabelText, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_HSLIDER || WIDGET_VSLIDER
    connect(this, &SIM::setSliderValue, ui, &UI::onSetSliderValue, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_CHECKBOX
    connect(this, &SIM::setCheckboxValue, ui, &UI::onSetCheckboxValue, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_RADIOBUTTON
    connect(this, &SIM::setRadiobuttonValue, ui, &UI::onSetRadiobuttonValue, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_COMBOBOX
    connect(this, &SIM::insertComboboxItem, ui, &UI::onInsertComboboxItem, Qt::BlockingQueuedConnection);
    connect(this, &SIM::removeComboboxItem, ui, &UI::onRemoveComboboxItem, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setComboboxItems, ui, &UI::onSetComboboxItems, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setComboboxSelectedIndex, ui, &UI::onSetComboboxSelectedIndex, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_TABS
    connect(this, &SIM::setCurrentTab, ui, &UI::onSetCurrentTab, Qt::BlockingQueuedConnection);
#endif
    connect(this, &SIM::setWidgetVisibility, ui, &UI::onSetWidgetVisibility, Qt::BlockingQueuedConnection);
#if WIDGET_PLOT
    connect(this, &SIM::replot, ui, &UI::onReplot, Qt::BlockingQueuedConnection);
    connect(this, &SIM::addCurve, ui, &UI::onAddCurve, Qt::BlockingQueuedConnection);
    connect(this, &SIM::addCurveTimePoints, ui, &UI::onAddCurveTimePoints, Qt::BlockingQueuedConnection);
    connect(this, &SIM::addCurveXYPoints, ui, &UI::onAddCurveXYPoints, Qt::BlockingQueuedConnection);
    connect(this, &SIM::clearCurve, ui, &UI::onClearCurve, Qt::BlockingQueuedConnection);
    connect(this, &SIM::removeCurve, ui, &UI::onRemoveCurve, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setPlotRanges, ui, &UI::onSetPlotRanges, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setPlotXRange, ui, &UI::onSetPlotXRange, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setPlotYRange, ui, &UI::onSetPlotYRange, Qt::BlockingQueuedConnection);
    connect(this, &SIM::growPlotRanges, ui, &UI::onGrowPlotRanges, Qt::BlockingQueuedConnection);
    connect(this, &SIM::growPlotXRange, ui, &UI::onGrowPlotXRange, Qt::BlockingQueuedConnection);
    connect(this, &SIM::growPlotYRange, ui, &UI::onGrowPlotYRange, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setPlotLabels, ui, &UI::onSetPlotLabels, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setPlotXLabel, ui, &UI::onSetPlotXLabel, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setPlotYLabel, ui, &UI::onSetPlotYLabel, Qt::BlockingQueuedConnection);
    connect(this, &SIM::rescaleAxes, ui, &UI::onRescaleAxes, Qt::BlockingQueuedConnection);
    connect(this, &SIM::rescaleAxesAll, ui, &UI::onRescaleAxesAll, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setMouseOptions, ui, &UI::onSetMouseOptions, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setLegendVisibility, ui, &UI::onSetLegendVisibility, Qt::BlockingQueuedConnection);
    connect(ui, &UI::plottableClick, this, &SIM::onPlottableClick);
    connect(ui, &UI::legendClick, this, &SIM::onLegendClick);
#endif
#if WIDGET_TABLE
    connect(ui, &UI::cellActivate, this, &SIM::onCellActivate);
    connect(ui, &UI::tableSelectionChange, this, &SIM::onSelectionChangeTable);
#endif
#if WIDGET_PROPERTIES
    connect(ui, &UI::propertiesSelectionChange, this, &SIM::onSelectionChangeProperties);
    connect(ui, &UI::propertiesDoubleClick, this, &SIM::onPropertiesDoubleClick);
#endif
#if WIDGET_TREE
    connect(ui, &UI::treeSelectionChange, this, &SIM::onSelectionChangeTree);
#endif
#if WIDGET_IMAGE
    connect(ui, &UI::mouseEvent, this, &SIM::onMouseEvent);
#endif
#if WIDGET_TABLE
    connect(this, &SIM::clearTable, ui, &UI::onClearTable, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setRowCount, ui, &UI::onSetRowCount, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setColumnCountTable, ui, &UI::onSetColumnCountTable, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_TREE
    connect(this, &SIM::setColumnCountTree, ui, &UI::onSetColumnCountTree, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_TABLE
    connect(this, &SIM::setItem, ui, &UI::onSetItem, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setItemImage, ui, &UI::onSetItemImage, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setItems, ui, &UI::onSetItems, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setRowHeaderText, ui, &UI::onSetRowHeaderText, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setColumnHeaderTextTable, ui, &UI::onSetColumnHeaderTextTable, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_TREE
    connect(this, &SIM::setColumnHeaderTextTree, ui, &UI::onSetColumnHeaderTextTree, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_TABLE
    connect(this, &SIM::setItemEditable, ui, &UI::onSetItemEditable, Qt::BlockingQueuedConnection);
    connect(this, &SIM::restoreStateTable, ui, &UI::onRestoreStateTable, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_TREE
    connect(this, &SIM::restoreStateTree, ui, &UI::onRestoreStateTree, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_TABLE
    connect(this, &SIM::setRowHeight, ui, &UI::onSetRowHeight, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setColumnWidthTable, ui, &UI::onSetColumnWidthTable, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_TREE
    connect(this, &SIM::setColumnWidthTree, ui, &UI::onSetColumnWidthTree, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_TABLE
    connect(this, &SIM::setTableSelection, ui, &UI::onSetTableSelection, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_PROGRESSBAR
    connect(this, &SIM::setProgress, ui, &UI::onSetProgress, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_PROPERTIES
    connect(this, &SIM::setProperties, ui, &UI::onSetProperties, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setPropertiesRow, ui, &UI::onSetPropertiesRow, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setPropertiesSelection, ui, &UI::onSetPropertiesSelection, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_TREE
    connect(this, &SIM::clearTree, ui, &UI::onClearTree, Qt::BlockingQueuedConnection);
    connect(this, &SIM::addTreeItem, ui, &UI::onAddTreeItem, Qt::BlockingQueuedConnection);
    connect(this, &SIM::updateTreeItemText, ui, &UI::onUpdateTreeItemText, Qt::BlockingQueuedConnection);
    connect(this, &SIM::updateTreeItemParent, ui, &UI::onUpdateTreeItemParent, Qt::BlockingQueuedConnection);
    connect(this, &SIM::removeTreeItem, ui, &UI::onRemoveTreeItem, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setTreeSelection, ui, &UI::onSetTreeSelection, Qt::BlockingQueuedConnection);
    connect(this, &SIM::expandAll, ui, &UI::onExpandAll, Qt::BlockingQueuedConnection);
    connect(this, &SIM::collapseAll, ui, &UI::onCollapseAll, Qt::BlockingQueuedConnection);
    connect(this, &SIM::expandToDepth, ui, &UI::onExpandToDepth, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_TEXTBROWSER
    connect(this, &SIM::setText, ui, &UI::onSetText, Qt::BlockingQueuedConnection);
    connect(this, &SIM::appendText, ui, &UI::onAppendText, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setUrl, ui, &UI::onSetUrl, Qt::BlockingQueuedConnection);
#endif
    connect(ui, &UI::keyPressed, this, &SIM::onKeyPress);
#if WIDGET_SCENE3D
    connect(this, &SIM::addScene3DNode, ui, &UI::onAddScene3DNode, Qt::BlockingQueuedConnection);
    connect(this, &SIM::removeScene3DNode, ui, &UI::onRemoveScene3DNode, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setScene3DNodeEnabled, ui, &UI::onSetScene3DNodeEnabled, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setScene3DIntParam, ui, &UI::onSetScene3DIntParam, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setScene3DFloatParam, ui, &UI::onSetScene3DFloatParam, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setScene3DStringParam, ui, &UI::onSetScene3DStringParam, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setScene3DVector2Param, ui, &UI::onSetScene3DVector2Param, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setScene3DVector3Param, ui, &UI::onSetScene3DVector3Param, Qt::BlockingQueuedConnection);
    connect(this, &SIM::setScene3DVector4Param, ui, &UI::onSetScene3DVector4Param, Qt::BlockingQueuedConnection);
    connect(ui, &UI::scene3DObjectClick, this, &SIM::onScene3DObjectClick);
#endif
#if WIDGET_SVG
    connect(this, &SIM::svgLoadFile, ui, &UI::onSvgLoadFile, Qt::BlockingQueuedConnection);
    connect(this, &SIM::svgLoadData, ui, &UI::onSvgLoadData, Qt::BlockingQueuedConnection);
#endif
}

/**
 * while events are delivered, objects may be deleted in the other thread.
 * (this can happen when stopping the simulation for instance).
 * in order to prevent a crash, we check with Widget/Window::exists(..) if the
 * pointer refers to an object which is still valid (i.e. not deleted)
 */
#define CHECK_POINTER(clazz,p) \
    if(!p) return; \
    if(!clazz::exists(p)) {sim::addLog(sim_verbosity_warnings, "%s %x has already been deleted (or the pointer is invalid)", #clazz, p); return;} \
    if(!p->proxy) return;

#if WIDGET_BUTTON
void SIM::onButtonClick(Widget *widget)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, widget);

    EventOnClick *e = dynamic_cast<EventOnClick*>(widget);

    if(!e) return;
    if(e->onclick == "" || widget->proxy->scriptID == -1) return;

    onclickCallback_in in;
    in.handle = widget->proxy->handle;
    in.id = widget->id;
    onclickCallback_out out;
    onclickCallback(widget->proxy->scriptID, e->onclick.c_str(), &in, &out);
}
#endif

#if WIDGET_LABEL
void SIM::onLinkActivated(Widget *widget, QString link)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, widget);

    EventOnLinkActivated *e = dynamic_cast<EventOnLinkActivated*>(widget);

    if(!e) return;
    if(e->onLinkActivated == "" || widget->proxy->scriptID == -1) return;

    onLinkActivatedCallback_in in;
    in.handle = widget->proxy->handle;
    in.id = widget->id;
    in.link = link.toStdString();
    onLinkActivatedCallback_out out;
    onLinkActivatedCallback(widget->proxy->scriptID, e->onLinkActivated.c_str(), &in, &out);
}
#endif

void SIM::onValueChangeInt(Widget *widget, int value)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, widget);

    EventOnChangeInt *ei = dynamic_cast<EventOnChangeInt*>(widget);
    EventOnChangeDouble *ed = 0L;
#if WIDGET_SPINBOX
    /* XXX: spinbox inherits EventOnChangeDouble; however, when float="false",
     *      it emits a valueChanged(int) signal. So we try to read here also
     *      for a EventOnChangeDouble handler here to cover for that case.
     */
    ed = dynamic_cast<Spinbox*>(widget) ? dynamic_cast<EventOnChangeDouble*>(widget) : 0;
#endif

    if(!ei && !ed) return;
    std::string onchange = ei ? ei->onchange : ed->onchange;
    if(onchange == "" || widget->proxy->scriptID == -1) return;

    onchangeIntCallback_in in;
    in.handle = widget->proxy->handle;
    in.id = widget->id;
    in.value = value;
    onchangeIntCallback_out out;
    onchangeIntCallback(widget->proxy->scriptID, onchange.c_str(), &in, &out);
}

void SIM::onValueChangeDouble(Widget *widget, double value)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, widget);

    EventOnChangeDouble *e = dynamic_cast<EventOnChangeDouble*>(widget);

    if(!e) return;
    if(e->onchange == "" || widget->proxy->scriptID == -1) return;

    onchangeDoubleCallback_in in;
    in.handle = widget->proxy->handle;
    in.id = widget->id;
    in.value = value;
    onchangeDoubleCallback_out out;
    onchangeDoubleCallback(widget->proxy->scriptID, e->onchange.c_str(), &in, &out);
}

void SIM::onValueChangeString(Widget *widget, QString value)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, widget);

    EventOnChangeString *e = dynamic_cast<EventOnChangeString*>(widget);

    if(!e) return;
    if(e->onchange == "" || widget->proxy->scriptID == -1) return;

    onchangeStringCallback_in in;
    in.handle = widget->proxy->handle;
    in.id = widget->id;
    in.value = value.toStdString();
    onchangeStringCallback_out out;
    onchangeStringCallback(widget->proxy->scriptID, e->onchange.c_str(), &in, &out);
}

#if WIDGET_EDIT
void SIM::onEditingFinished(Edit *edit, QString value)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, edit);

    EventOnEditingFinished *e = dynamic_cast<EventOnEditingFinished*>(edit);

    if(!e) return;
    if(e->oneditingfinished == "" || edit->proxy->scriptID == -1) return;

    oneditingfinishedCallback_in in;
    in.handle = edit->proxy->handle;
    in.id = edit->id;
    in.value = value.toStdString();
    oneditingfinishedCallback_out out;
    oneditingfinishedCallback(edit->proxy->scriptID, e->oneditingfinished.c_str(), &in, &out);
}

void SIM::onEvaluateExpressionInSandbox(Edit *edit, QString expr)
{
    QString code = QString("tostring(load([===[return %1]===])())@lua").arg(expr);
    int stack = sim::createStack();
    try
    {
        sim::executeScriptString(sim::getScriptHandleEx(sim_scripttype_sandbox), code.toStdString(), stack);
        std::string result;
        sim::getStackStringValue(stack, &result);
        emit setEvaluationResult(edit, QString::fromStdString(result));
    }
    catch(std::exception &ex)
    {
        std::string err;
        sim::getStackStringValue(stack, &err);
        //sim::addLog(sim_verbosity_warnings, "eval error: " + err);
    }
    sim::releaseStack(stack);
}
#endif

void SIM::onWindowClose(Window *window)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Window, window);

    oncloseCallback_in in;
    in.handle = window->proxy->handle;
    oncloseCallback_out out;
    oncloseCallback(window->proxy->getScriptID(), window->onclose.c_str(), &in, &out);
}

#if WIDGET_IMAGE
void SIM::onLoadImageFromFile(Image *image, const char *filename, int w, int h)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, image);

    int resolution[2];
    unsigned char *data = nullptr;
    try
    {
        data = sim::loadImage(resolution, 0, filename, nullptr);
    }
    catch(sim::api_error &ex)
    {
        sim::addLog(sim_verbosity_errors, "Failed to load image %s: %s", filename, ex.what());
        return;
    }
    sim::transformImage(data, resolution, 4);

    if(w > 0 && h > 0)
    {
        int size[2] = {w, h};
        unsigned char *scaled = sim::getScaledImage(data, resolution, size, 0);
        sim::releaseBuffer((char *)data);
        setImage(image, (char *)scaled, w, h);
    }
    else
    {
        setImage(image, (char *)data, resolution[0], resolution[1]);
    }
}
#endif

#if WIDGET_PLOT
void SIM::onPlottableClick(Plot *plot, std::string name, int index, double x, double y)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, plot);

    if(plot->onCurveClick == "" || plot->proxy->scriptID == -1) return;

    onPlottableClickCallback_in in;
    in.handle = plot->proxy->handle;
    in.id = plot->id;
    in.curve = name;
    in.index = index;
    in.x = x;
    in.y = y;
    onPlottableClickCallback_out out;
    onPlottableClickCallback(plot->proxy->getScriptID(), plot->onCurveClick.c_str(), &in, &out);
}

void SIM::onLegendClick(Plot *plot, std::string name)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, plot);

    if(plot->onLegendClick == "" || plot->proxy->scriptID == -1) return;

    onLegendClickCallback_in in;
    in.handle = plot->proxy->handle;
    in.id = plot->id;
    in.curve = name;
    onLegendClickCallback_out out;
    onLegendClickCallback(plot->proxy->getScriptID(), plot->onLegendClick.c_str(), &in, &out);
}
#endif

#if WIDGET_TABLE
void SIM::onCellActivate(Table *table, int row, int col, std::string text)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, table);

    if(table->onCellActivate == "" || table->proxy->scriptID == -1) return;

    onCellActivateCallback_in in;
    in.handle = table->proxy->handle;
    in.id = table->id;
    in.row = row;
    in.column = col;
    in.cellValue = text;
    onCellActivateCallback_out out;
    onCellActivateCallback(table->proxy->getScriptID(), table->onCellActivate.c_str(), &in, &out);
}

void SIM::onSelectionChangeTable(Table *table, int row, int col)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, table);

    if(table->onSelectionChange == "" || table->proxy->scriptID == -1) return;

    onTableSelectionChangeCallback_in in;
    in.handle = table->proxy->handle;
    in.id = table->id;
    in.row = row;
    in.column = col;
    onTableSelectionChangeCallback_out out;
    onTableSelectionChangeCallback(table->proxy->getScriptID(), table->onSelectionChange.c_str(), &in, &out);
}
#endif

#if WIDGET_PROPERTIES
void SIM::onSelectionChangeProperties(Properties *properties, int row)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, properties);

    if(properties->onSelectionChange == "" || properties->proxy->scriptID == -1) return;

    onPropertiesSelectionChangeCallback_in in;
    in.handle = properties->proxy->handle;
    in.id = properties->id;
    in.row = row;
    onPropertiesSelectionChangeCallback_out out;
    onPropertiesSelectionChangeCallback(properties->proxy->getScriptID(), properties->onSelectionChange.c_str(), &in, &out);
}

void SIM::onPropertiesDoubleClick(Properties *properties, int row, int col)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, properties);

    if(properties->ondoubleclick == "" || properties->proxy->scriptID == -1) return;

    onPropertiesDoubleClickCallback_in in;
    in.handle = properties->proxy->handle;
    in.id = properties->id;
    in.row = row;
    in.col = col;
    onPropertiesDoubleClickCallback_out out;
    onPropertiesDoubleClickCallback(properties->proxy->getScriptID(), properties->ondoubleclick.c_str(), &in, &out);
}
#endif

#if WIDGET_TREE
void SIM::onSelectionChangeTree(Tree *tree, int id)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, tree);

    if(tree->onSelectionChange == "" || tree->proxy->scriptID == -1) return;

    onTreeSelectionChangeCallback_in in;
    in.handle = tree->proxy->handle;
    in.id = tree->id;
    in.item_id = id;
    onTreeSelectionChangeCallback_out out;
    onTreeSelectionChangeCallback(tree->proxy->getScriptID(), tree->onSelectionChange.c_str(), &in, &out);
}
#endif

#if WIDGET_IMAGE || WIDGET_SVG
void SIM::onMouseEvent(Widget *widget, int type, bool shift, bool control, int x, int y)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, widget);

    if(widget->proxy->scriptID == -1) return;

    onMouseEventCallback_in in;
    in.handle = widget->proxy->handle;
    in.id = widget->id;
    in.type = type;
    in.mods.shift = shift;
    in.mods.control = control;
    in.x = x;
    in.y = y;
    onMouseEventCallback_out out;
    std::string cb = "";
    switch(type)
    {
    case simui_mouse_left_button_down:
        if(auto *eDown = dynamic_cast<EventOnMouseDown*>(widget))
            cb = eDown->onMouseDown;
        break;
    case simui_mouse_left_button_up:
        if(auto *eUp = dynamic_cast<EventOnMouseUp*>(widget))
            cb = eUp->onMouseUp;
        break;
    case simui_mouse_move:
        if(auto *eMove = dynamic_cast<EventOnMouseMove*>(widget))
            cb = eMove->onMouseMove;
        break;
    }
    if(cb == "") return;
    onMouseEventCallback(widget->proxy->getScriptID(), cb.c_str(), &in, &out);
}
#endif

void SIM::onKeyPress(Widget *widget, int key, std::string text)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, widget);

    if(widget->proxy->scriptID == -1) return;

    EventOnKeyPress *e = dynamic_cast<EventOnKeyPress*>(widget);
    if(!e) return;
    if(e->onKeyPress == "") return;

    onKeyPressCallback_in in;
    in.handle = widget->proxy->handle;
    in.id = widget->id;
    in.key = key;
    in.text = text;
    onKeyPressCallback_out out;
    onKeyPressCallback(widget->proxy->getScriptID(), e->onKeyPress.c_str(), &in, &out);
}

#if WIDGET_SCENE3D
void SIM::onScene3DObjectClick(Scene3D *scene3d, int id)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, scene3d);

    if(scene3d->proxy->scriptID == -1) return;

    if(scene3d->onClick == "") return;

    onScene3DObjectClickCallback_in in;
    in.handle = scene3d->proxy->handle;
    in.id = scene3d->id;
    in.nodeId = id;
    onScene3DObjectClickCallback_out out;
    onScene3DObjectClickCallback(scene3d->proxy->getScriptID(), scene3d->onClick.c_str(), &in, &out);
}
#endif

