#include "UIFunctions.h"
#include "UIProxy.h"

#include <QThread>

#include <iostream>

#include <simPlusPlus/Lib.h>
#include "stubs.h"

// UIFunctions is a singleton

UIFunctions *UIFunctions::instance = NULL;

UIFunctions::UIFunctions(QObject *parent)
    : QObject(parent)
{
    connectSignals();
}

UIFunctions::~UIFunctions()
{
    UIFunctions::instance = NULL;
}

UIFunctions * UIFunctions::getInstance(QObject *parent)
{
    if(!UIFunctions::instance)
    {
        UIFunctions::instance = new UIFunctions(parent);

        simThread(); // we remember of this currentThreadId as the "SIM" thread

        sim::addLog(sim_verbosity_debug, "UIFunctions(%x) constructed in thread %s", UIFunctions::instance, QThread::currentThreadId());
    }
    return UIFunctions::instance;
}

void UIFunctions::destroyInstance()
{
    TRACE_FUNC;

    if(UIFunctions::instance)
    {
        delete UIFunctions::instance;

        sim::addLog(sim_verbosity_debug, "destroyed UIFunctions instance");
    }
}

void UIFunctions::connectSignals()
{
    UIProxy *uiproxy = UIProxy::getInstance();
    // connect signals/slots from UIProxy to UIFunctions and vice-versa
    connect(this, &UIFunctions::msgBox, uiproxy, &UIProxy::onMsgBox, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::fileDialog, uiproxy, &UIProxy::onFileDialog, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::colorDialog, uiproxy, &UIProxy::onColorDialog, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::create, uiproxy, &UIProxy::onCreate, Qt::BlockingQueuedConnection);
#if WIDGET_BUTTON
    connect(uiproxy, &UIProxy::buttonClick, this, &UIFunctions::onButtonClick);
#endif
#if WIDGET_LABEL
    connect(uiproxy, &UIProxy::linkActivated, this, &UIFunctions::onLinkActivated);
#endif
    connect(uiproxy, &UIProxy::valueChangeInt, this, &UIFunctions::onValueChangeInt);
    connect(uiproxy, &UIProxy::valueChangeDouble, this, &UIFunctions::onValueChangeDouble);
    connect(uiproxy, &UIProxy::valueChangeString, this, &UIFunctions::onValueChangeString);
#if WIDGET_EDIT
    connect(uiproxy, &UIProxy::editingFinished, this, &UIFunctions::onEditingFinished);
#endif
    connect(uiproxy, &UIProxy::windowClose, this, &UIFunctions::onWindowClose);
    connect(this, &UIFunctions::destroy, uiproxy, &UIProxy::onDestroy, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setStyleSheet, uiproxy, &UIProxy::onSetStyleSheet, Qt::BlockingQueuedConnection);
#if WIDGET_BUTTON
    connect(this, &UIFunctions::setButtonText, uiproxy, &UIProxy::onSetButtonText, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setButtonPressed, uiproxy, &UIProxy::onSetButtonPressed, Qt::BlockingQueuedConnection);
#endif
    connect(this, &UIFunctions::showWindow, uiproxy, &UIProxy::onShowWindow, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::hideWindow, uiproxy, &UIProxy::onHideWindow, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setPosition, uiproxy, &UIProxy::onSetPosition, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setSize, uiproxy, &UIProxy::onSetSize, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setTitle, uiproxy, &UIProxy::onSetTitle, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setWindowEnabled, uiproxy, &UIProxy::onSetWindowEnabled, Qt::BlockingQueuedConnection);
#if WIDGET_IMAGE
    connect(this, &UIFunctions::setImage, uiproxy, &UIProxy::onSetImage, Qt::BlockingQueuedConnection);
    connect(uiproxy, &UIProxy::loadImageFromFile, this, &UIFunctions::onLoadImageFromFile);
#endif
    connect(this, &UIFunctions::sceneChange, uiproxy, &UIProxy::onSceneChange, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setEnabled, uiproxy, &UIProxy::onSetEnabled, Qt::BlockingQueuedConnection);
#if WIDGET_EDIT
    connect(this, &UIFunctions::setEditValue, uiproxy, &UIProxy::onSetEditValue, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_SPINBOX
    connect(this, &UIFunctions::setSpinboxValue, uiproxy, &UIProxy::onSetSpinboxValue, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_LABEL
    connect(this, &UIFunctions::setLabelText, uiproxy, &UIProxy::onSetLabelText, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_HSLIDER || WIDGET_VSLIDER
    connect(this, &UIFunctions::setSliderValue, uiproxy, &UIProxy::onSetSliderValue, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_CHECKBOX
    connect(this, &UIFunctions::setCheckboxValue, uiproxy, &UIProxy::onSetCheckboxValue, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_RADIOBUTTON
    connect(this, &UIFunctions::setRadiobuttonValue, uiproxy, &UIProxy::onSetRadiobuttonValue, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_COMBOBOX
    connect(this, &UIFunctions::insertComboboxItem, uiproxy, &UIProxy::onInsertComboboxItem, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::removeComboboxItem, uiproxy, &UIProxy::onRemoveComboboxItem, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setComboboxItems, uiproxy, &UIProxy::onSetComboboxItems, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setComboboxSelectedIndex, uiproxy, &UIProxy::onSetComboboxSelectedIndex, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_TABS
    connect(this, &UIFunctions::setCurrentTab, uiproxy, &UIProxy::onSetCurrentTab, Qt::BlockingQueuedConnection);
#endif
    connect(this, &UIFunctions::setWidgetVisibility, uiproxy, &UIProxy::onSetWidgetVisibility, Qt::BlockingQueuedConnection);
#if WIDGET_PLOT
    connect(this, &UIFunctions::replot, uiproxy, &UIProxy::onReplot, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::addCurve, uiproxy, &UIProxy::onAddCurve, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::addCurveTimePoints, uiproxy, &UIProxy::onAddCurveTimePoints, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::addCurveXYPoints, uiproxy, &UIProxy::onAddCurveXYPoints, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::clearCurve, uiproxy, &UIProxy::onClearCurve, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::removeCurve, uiproxy, &UIProxy::onRemoveCurve, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setPlotRanges, uiproxy, &UIProxy::onSetPlotRanges, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setPlotXRange, uiproxy, &UIProxy::onSetPlotXRange, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setPlotYRange, uiproxy, &UIProxy::onSetPlotYRange, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::growPlotRanges, uiproxy, &UIProxy::onGrowPlotRanges, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::growPlotXRange, uiproxy, &UIProxy::onGrowPlotXRange, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::growPlotYRange, uiproxy, &UIProxy::onGrowPlotYRange, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setPlotLabels, uiproxy, &UIProxy::onSetPlotLabels, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setPlotXLabel, uiproxy, &UIProxy::onSetPlotXLabel, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setPlotYLabel, uiproxy, &UIProxy::onSetPlotYLabel, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::rescaleAxes, uiproxy, &UIProxy::onRescaleAxes, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::rescaleAxesAll, uiproxy, &UIProxy::onRescaleAxesAll, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setMouseOptions, uiproxy, &UIProxy::onSetMouseOptions, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setLegendVisibility, uiproxy, &UIProxy::onSetLegendVisibility, Qt::BlockingQueuedConnection);
    connect(uiproxy, &UIProxy::plottableClick, this, &UIFunctions::onPlottableClick);
    connect(uiproxy, &UIProxy::legendClick, this, &UIFunctions::onLegendClick);
#endif
#if WIDGET_TABLE
    connect(uiproxy, &UIProxy::cellActivate, this, &UIFunctions::onCellActivate);
    connect(uiproxy, &UIProxy::tableSelectionChange, this, &UIFunctions::onSelectionChangeTable);
#endif
#if WIDGET_TREE
    connect(uiproxy, &UIProxy::treeSelectionChange, this, &UIFunctions::onSelectionChangeTree);
#endif
#if WIDGET_IMAGE
    connect(uiproxy, &UIProxy::mouseEvent, this, &UIFunctions::onMouseEvent);
#endif
#if WIDGET_TABLE
    connect(this, &UIFunctions::clearTable, uiproxy, &UIProxy::onClearTable, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setRowCount, uiproxy, &UIProxy::onSetRowCount, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setColumnCountTable, uiproxy, &UIProxy::onSetColumnCountTable, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_TREE
    connect(this, &UIFunctions::setColumnCountTree, uiproxy, &UIProxy::onSetColumnCountTree, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_TABLE
    connect(this, &UIFunctions::setItem, uiproxy, &UIProxy::onSetItem, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setItemImage, uiproxy, &UIProxy::onSetItemImage, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setRowHeaderText, uiproxy, &UIProxy::onSetRowHeaderText, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setColumnHeaderTextTable, uiproxy, &UIProxy::onSetColumnHeaderTextTable, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_TREE
    connect(this, &UIFunctions::setColumnHeaderTextTree, uiproxy, &UIProxy::onSetColumnHeaderTextTree, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_TABLE
    connect(this, &UIFunctions::setItemEditable, uiproxy, &UIProxy::onSetItemEditable, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::restoreStateTable, uiproxy, &UIProxy::onRestoreStateTable, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_TREE
    connect(this, &UIFunctions::restoreStateTree, uiproxy, &UIProxy::onRestoreStateTree, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_TABLE
    connect(this, &UIFunctions::setRowHeight, uiproxy, &UIProxy::onSetRowHeight, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setColumnWidthTable, uiproxy, &UIProxy::onSetColumnWidthTable, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_TREE
    connect(this, &UIFunctions::setColumnWidthTree, uiproxy, &UIProxy::onSetColumnWidthTree, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_TABLE
    connect(this, &UIFunctions::setTableSelection, uiproxy, &UIProxy::onSetTableSelection, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_PROGRESSBAR
    connect(this, &UIFunctions::setProgress, uiproxy, &UIProxy::onSetProgress, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_TREE
    connect(this, &UIFunctions::clearTree, uiproxy, &UIProxy::onClearTree, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::addTreeItem, uiproxy, &UIProxy::onAddTreeItem, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::updateTreeItemText, uiproxy, &UIProxy::onUpdateTreeItemText, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::updateTreeItemParent, uiproxy, &UIProxy::onUpdateTreeItemParent, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::removeTreeItem, uiproxy, &UIProxy::onRemoveTreeItem, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setTreeSelection, uiproxy, &UIProxy::onSetTreeSelection, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::expandAll, uiproxy, &UIProxy::onExpandAll, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::collapseAll, uiproxy, &UIProxy::onCollapseAll, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::expandToDepth, uiproxy, &UIProxy::onExpandToDepth, Qt::BlockingQueuedConnection);
#endif
#if WIDGET_DATAFLOW
    connect(this, &UIFunctions::addNode, uiproxy, &UIProxy::onAddNode, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::removeNode, uiproxy, &UIProxy::onRemoveNode, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setNodeValid, uiproxy, &UIProxy::onSetNodeValid, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setNodePos, uiproxy, &UIProxy::onSetNodePos, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setNodeText, uiproxy, &UIProxy::onSetNodeText, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setNodeInletCount, uiproxy, &UIProxy::onSetNodeInletCount, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setNodeOutletCount, uiproxy, &UIProxy::onSetNodeOutletCount, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::addConnection, uiproxy, &UIProxy::onAddConnection, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::removeConnection, uiproxy, &UIProxy::onRemoveConnection, Qt::BlockingQueuedConnection);
    connect(uiproxy, &UIProxy::nodeAdded, this, &UIFunctions::onNodeAdded);
    connect(uiproxy, &UIProxy::nodeRemoved, this, &UIFunctions::onNodeRemoved);
    connect(uiproxy, &UIProxy::nodeValidChanged, this, &UIFunctions::onNodeValidChanged);
    connect(uiproxy, &UIProxy::nodePosChanged, this, &UIFunctions::onNodePosChanged);
    connect(uiproxy, &UIProxy::nodeTextChanged, this, &UIFunctions::onNodeTextChanged);
    connect(uiproxy, &UIProxy::nodeInletCountChanged, this, &UIFunctions::onNodeInletCountChanged);
    connect(uiproxy, &UIProxy::nodeOutletCountChanged, this, &UIFunctions::onNodeOutletCountChanged);
    connect(uiproxy, &UIProxy::connectionAdded, this, &UIFunctions::onConnectionAdded);
    connect(uiproxy, &UIProxy::connectionRemoved, this, &UIFunctions::onConnectionRemoved);
#endif
#if WIDGET_TEXTBROWSER
    connect(this, &UIFunctions::setText, uiproxy, &UIProxy::onSetText, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setUrl, uiproxy, &UIProxy::onSetUrl, Qt::BlockingQueuedConnection);
#endif
    connect(uiproxy, &UIProxy::keyPressed, this, &UIFunctions::onKeyPress);
#if WIDGET_SCENE3D
    connect(this, &UIFunctions::addScene3DNode, uiproxy, &UIProxy::onAddScene3DNode, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::removeScene3DNode, uiproxy, &UIProxy::onRemoveScene3DNode, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setScene3DNodeEnabled, uiproxy, &UIProxy::onSetScene3DNodeEnabled, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setScene3DIntParam, uiproxy, &UIProxy::onSetScene3DIntParam, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setScene3DFloatParam, uiproxy, &UIProxy::onSetScene3DFloatParam, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setScene3DStringParam, uiproxy, &UIProxy::onSetScene3DStringParam, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setScene3DVector2Param, uiproxy, &UIProxy::onSetScene3DVector2Param, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setScene3DVector3Param, uiproxy, &UIProxy::onSetScene3DVector3Param, Qt::BlockingQueuedConnection);
    connect(this, &UIFunctions::setScene3DVector4Param, uiproxy, &UIProxy::onSetScene3DVector4Param, Qt::BlockingQueuedConnection);
    connect(uiproxy, &UIProxy::scene3DObjectClick, this, &UIFunctions::onScene3DObjectClick);
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
void UIFunctions::onButtonClick(Widget *widget)
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
void UIFunctions::onLinkActivated(Widget *widget, QString link)
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

void UIFunctions::onValueChangeInt(Widget *widget, int value)
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

void UIFunctions::onValueChangeDouble(Widget *widget, double value)
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

void UIFunctions::onValueChangeString(Widget *widget, QString value)
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
void UIFunctions::onEditingFinished(Edit *edit, QString value)
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
#endif

void UIFunctions::onWindowClose(Window *window)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Window, window);

    oncloseCallback_in in;
    in.handle = window->proxy->getHandle();
    oncloseCallback_out out;
    oncloseCallback(window->proxy->getScriptID(), window->onclose.c_str(), &in, &out);
}

#if WIDGET_IMAGE
void UIFunctions::onLoadImageFromFile(Image *image, const char *filename, int w, int h)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, image);

    int resolution[2];
    simUChar *data = simLoadImage(resolution, 0, filename, NULL);
    simTransformImage(data, resolution, 4, NULL, NULL, NULL);

    if(w > 0 && h > 0)
    {
        int size[2] = {w, h};
        simUChar *scaled = simGetScaledImage(data, resolution, size, 0, NULL);
        sim::releaseBuffer((simChar *)data);
        setImage(image, (simChar *)scaled, w, h);
    }
    else
    {
        setImage(image, (simChar *)data, resolution[0], resolution[1]);
    }
}
#endif

#if WIDGET_PLOT
void UIFunctions::onPlottableClick(Plot *plot, std::string name, int index, double x, double y)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, plot);

    if(plot->onCurveClick == "" || plot->proxy->scriptID == -1) return;

    onPlottableClickCallback_in in;
    in.handle = plot->proxy->getHandle();
    in.id = plot->id;
    in.curve = name;
    in.index = index;
    in.x = x;
    in.y = y;
    onPlottableClickCallback_out out;
    onPlottableClickCallback(plot->proxy->getScriptID(), plot->onCurveClick.c_str(), &in, &out);
}

void UIFunctions::onLegendClick(Plot *plot, std::string name)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, plot);

    if(plot->onLegendClick == "" || plot->proxy->scriptID == -1) return;

    onLegendClickCallback_in in;
    in.handle = plot->proxy->getHandle();
    in.id = plot->id;
    in.curve = name;
    onLegendClickCallback_out out;
    onLegendClickCallback(plot->proxy->getScriptID(), plot->onLegendClick.c_str(), &in, &out);
}
#endif

#if WIDGET_TABLE
void UIFunctions::onCellActivate(Table *table, int row, int col, std::string text)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, table);

    if(table->onCellActivate == "" || table->proxy->scriptID == -1) return;

    onCellActivateCallback_in in;
    in.handle = table->proxy->getHandle();
    in.id = table->id;
    in.row = row;
    in.column = col;
    in.cellValue = text;
    onCellActivateCallback_out out;
    onCellActivateCallback(table->proxy->getScriptID(), table->onCellActivate.c_str(), &in, &out);
}

void UIFunctions::onSelectionChangeTable(Table *table, int row, int col)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, table);

    if(table->onSelectionChange == "" || table->proxy->scriptID == -1) return;

    onTableSelectionChangeCallback_in in;
    in.handle = table->proxy->getHandle();
    in.id = table->id;
    in.row = row;
    in.column = col;
    onTableSelectionChangeCallback_out out;
    onTableSelectionChangeCallback(table->proxy->getScriptID(), table->onSelectionChange.c_str(), &in, &out);
}
#endif

#if WIDGET_TREE
void UIFunctions::onSelectionChangeTree(Tree *tree, int id)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, tree);

    if(tree->onSelectionChange == "" || tree->proxy->scriptID == -1) return;

    onTreeSelectionChangeCallback_in in;
    in.handle = tree->proxy->getHandle();
    in.id = tree->id;
    in.item_id = id;
    onTreeSelectionChangeCallback_out out;
    onTreeSelectionChangeCallback(tree->proxy->getScriptID(), tree->onSelectionChange.c_str(), &in, &out);
}
#endif

#if WIDGET_IMAGE
void UIFunctions::onMouseEvent(Image *image, int type, bool shift, bool control, int x, int y)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, image);

    if(image->proxy->scriptID == -1) return;

    onMouseEventCallback_in in;
    in.handle = image->proxy->getHandle();
    in.id = image->id;
    in.type = type;
    in.mods.shift = shift;
    in.mods.control = control;
    in.x = x;
    in.y = y;
    onMouseEventCallback_out out;
    std::string cb = "";
    switch(type)
    {
    case sim_customui_mouse_left_button_down:
        cb = image->onMouseDown;
        break;
    case sim_customui_mouse_left_button_up:
        cb = image->onMouseUp;
        break;
    case sim_customui_mouse_move:
        cb = image->onMouseMove;
        break;
    }
    if(cb == "") return;
    onMouseEventCallback(image->proxy->getScriptID(), cb.c_str(), &in, &out);
}
#endif

#if WIDGET_DATAFLOW
void UIFunctions::onNodeAdded(Dataflow *dataflow, int id, QPoint pos, QString text, int inlets, int outlets)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, dataflow);

    if(dataflow->proxy->scriptID == -1) return;
    if(dataflow->onNodeAdded == "") return;

    onNodeAddedCallback_in in;
    in.handle = dataflow->proxy->getHandle();
    in.id = dataflow->id;
    in.nodeId = id;
    in.x = pos.x();
    in.y = pos.y();
    in.text = text.toStdString();
    in.inlets = inlets;
    in.outlets = outlets;
    onNodeAddedCallback_out out;
    onNodeAddedCallback(dataflow->proxy->getScriptID(), dataflow->onNodeAdded.c_str(), &in, &out);
}

void UIFunctions::onNodeRemoved(Dataflow *dataflow, int id)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, dataflow);

    if(dataflow->proxy->scriptID == -1) return;
    if(dataflow->onNodeRemoved == "") return;

    onNodeRemovedCallback_in in;
    in.handle = dataflow->proxy->getHandle();
    in.id = dataflow->id;
    in.nodeId = id;
    onNodeRemovedCallback_out out;
    onNodeRemovedCallback(dataflow->proxy->getScriptID(), dataflow->onNodeRemoved.c_str(), &in, &out);
}

void UIFunctions::onNodeValidChanged(Dataflow *dataflow, int id, bool valid)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, dataflow);

    if(dataflow->proxy->scriptID == -1) return;
    if(dataflow->onNodeValidChanged == "") return;

    onNodeValidChangedCallback_in in;
    in.handle = dataflow->proxy->getHandle();
    in.id = dataflow->id;
    in.nodeId = id;
    in.valid = valid;
    onNodeValidChangedCallback_out out;
    onNodeValidChangedCallback(dataflow->proxy->getScriptID(), dataflow->onNodeValidChanged.c_str(), &in, &out);
}

void UIFunctions::onNodePosChanged(Dataflow *dataflow, int id, QPoint pos)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, dataflow);

    if(dataflow->proxy->scriptID == -1) return;
    if(dataflow->onNodePosChanged == "") return;

    onNodePosChangedCallback_in in;
    in.handle = dataflow->proxy->getHandle();
    in.id = dataflow->id;
    in.nodeId = id;
    in.x = pos.x();
    in.y = pos.x();
    onNodePosChangedCallback_out out;
    onNodePosChangedCallback(dataflow->proxy->getScriptID(), dataflow->onNodePosChanged.c_str(), &in, &out);
}

void UIFunctions::onNodeTextChanged(Dataflow *dataflow, int id, QString text)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, dataflow);

    if(dataflow->proxy->scriptID == -1) return;
    if(dataflow->onNodeTextChanged == "") return;

    onNodeTextChangedCallback_in in;
    in.handle = dataflow->proxy->getHandle();
    in.id = dataflow->id;
    in.nodeId = id;
    in.text = text.toStdString();
    onNodeTextChangedCallback_out out;
    onNodeTextChangedCallback(dataflow->proxy->getScriptID(), dataflow->onNodeTextChanged.c_str(), &in, &out);
}

void UIFunctions::onNodeInletCountChanged(Dataflow *dataflow, int id, int inlets)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, dataflow);

    if(dataflow->proxy->scriptID == -1) return;
    if(dataflow->onNodeInletCountChanged == "") return;

    onNodeInletCountChangedCallback_in in;
    in.handle = dataflow->proxy->getHandle();
    in.id = dataflow->id;
    in.nodeId = id;
    in.count = inlets;
    onNodeInletCountChangedCallback_out out;
    onNodeInletCountChangedCallback(dataflow->proxy->getScriptID(), dataflow->onNodeInletCountChanged.c_str(), &in, &out);
}

void UIFunctions::onNodeOutletCountChanged(Dataflow *dataflow, int id, int outlets)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, dataflow);

    if(dataflow->proxy->scriptID == -1) return;
    if(dataflow->onNodeOutletCountChanged == "") return;

    onNodeOutletCountChangedCallback_in in;
    in.handle = dataflow->proxy->getHandle();
    in.id = dataflow->id;
    in.nodeId = id;
    in.count = outlets;
    onNodeOutletCountChangedCallback_out out;
    onNodeOutletCountChangedCallback(dataflow->proxy->getScriptID(), dataflow->onNodeOutletCountChanged.c_str(), &in, &out);
}

void UIFunctions::onConnectionAdded(Dataflow *dataflow, int srcNodeId, int srcOutlet, int dstNodeId, int dstInlet)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, dataflow);

    if(dataflow->proxy->scriptID == -1) return;
    if(dataflow->onConnectionAdded == "") return;

    onConnectionAddedCallback_in in;
    in.handle = dataflow->proxy->getHandle();
    in.id = dataflow->id;
    in.srcNodeId = srcNodeId;
    in.srcOutlet = srcOutlet;
    in.dstNodeId = dstNodeId;
    in.dstInlet = dstInlet;
    onConnectionAddedCallback_out out;
    onConnectionAddedCallback(dataflow->proxy->getScriptID(), dataflow->onConnectionAdded.c_str(), &in, &out);
}

void UIFunctions::onConnectionRemoved(Dataflow *dataflow, int srcNodeId, int srcOutlet, int dstNodeId, int dstInlet)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, dataflow);

    if(dataflow->proxy->scriptID == -1) return;
    if(dataflow->onConnectionRemoved == "") return;

    onConnectionRemovedCallback_in in;
    in.handle = dataflow->proxy->getHandle();
    in.id = dataflow->id;
    in.srcNodeId = srcNodeId;
    in.srcOutlet = srcOutlet;
    in.dstNodeId = dstNodeId;
    in.dstInlet = dstInlet;
    onConnectionRemovedCallback_out out;
    onConnectionRemovedCallback(dataflow->proxy->getScriptID(), dataflow->onConnectionRemoved.c_str(), &in, &out);
}
#endif

void UIFunctions::onKeyPress(Widget *widget, int key, std::string text)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, widget);

    if(widget->proxy->scriptID == -1) return;

    EventOnKeyPress *e = dynamic_cast<EventOnKeyPress*>(widget);
    if(!e) return;
    if(e->onKeyPress == "") return;

    onKeyPressCallback_in in;
    in.handle = widget->proxy->getHandle();
    in.id = widget->id;
    in.key = key;
    in.text = text;
    onKeyPressCallback_out out;
    onKeyPressCallback(widget->proxy->getScriptID(), e->onKeyPress.c_str(), &in, &out);
}

#if WIDGET_SCENE3D
void UIFunctions::onScene3DObjectClick(Scene3D *scene3d, int id)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, scene3d);

    if(scene3d->proxy->scriptID == -1) return;

    if(scene3d->onClick == "") return;

    onScene3DObjectClickCallback_in in;
    in.handle = scene3d->proxy->getHandle();
    in.id = scene3d->id;
    in.nodeId = id;
    onScene3DObjectClickCallback_out out;
    onScene3DObjectClickCallback(scene3d->proxy->getScriptID(), scene3d->onClick.c_str(), &in, &out);
}
#endif

