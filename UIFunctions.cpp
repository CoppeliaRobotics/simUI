#include "UIFunctions.h"
#include "debug.h"
#include "UIProxy.h"

#include <QThread>

#include <iostream>

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

        DBG << "UIFunctions(" << UIFunctions::instance << ") constructed in thread " << QThread::currentThreadId() << std::endl;
    }
    return UIFunctions::instance;
}

void UIFunctions::destroyInstance()
{
    DBG << "[enter]" << std::endl;

    if(UIFunctions::instance)
    {
        delete UIFunctions::instance;

        DBG << "destroyed UIFunctions instance" << std::endl;
    }

    DBG << "[leave]" << std::endl;
}

void UIFunctions::connectSignals()
{
    UIProxy *uiproxy = UIProxy::getInstance();
    // connect signals/slots from UIProxy to UIFunctions and vice-versa
    connect(this, SIGNAL(create(Proxy*)), uiproxy, SLOT(onCreate(Proxy*)), Qt::BlockingQueuedConnection);
    connect(uiproxy, SIGNAL(buttonClick(Widget*)), this, SLOT(onButtonClick(Widget*)));
    connect(uiproxy, SIGNAL(valueChange(Widget*,int)), this, SLOT(onValueChange(Widget*,int)));
    connect(uiproxy, SIGNAL(valueChange(Widget*,double)), this, SLOT(onValueChange(Widget*,double)));
    connect(uiproxy, SIGNAL(valueChange(Widget*,QString)), this, SLOT(onValueChange(Widget*,QString)));
    connect(uiproxy, SIGNAL(editingFinished(Edit*,QString)), this, SLOT(onEditingFinished(Edit*,QString)));
    connect(uiproxy, SIGNAL(windowClose(Window*)), this, SLOT(onWindowClose(Window*)));
    connect(this, SIGNAL(destroy(Proxy*)), uiproxy, SLOT(onDestroy(Proxy*)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(showWindow(Window*)), uiproxy, SLOT(onShowWindow(Window*)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(hideWindow(Window*)), uiproxy, SLOT(onHideWindow(Window*)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setPosition(Window*,int,int)), uiproxy, SLOT(onSetPosition(Window*,int,int)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setSize(Window*,int,int)), uiproxy, SLOT(onSetSize(Window*,int,int)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setTitle(Window*,std::string)), uiproxy, SLOT(onSetTitle(Window*,std::string)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setImage(Image*,const char*,int,int)), uiproxy, SLOT(onSetImage(Image*,const char*,int,int)), Qt::BlockingQueuedConnection);
    connect(uiproxy, SIGNAL(loadImageFromFile(Image*,const char *,int,int)), this, SLOT(onLoadImageFromFile(Image*,const char *,int,int)));
    connect(this, SIGNAL(sceneChange(Window*,int,int)), uiproxy, SLOT(onSceneChange(Window*,int,int)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setEnabled(Widget*,bool)), uiproxy, SLOT(onSetEnabled(Widget*,bool)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setEditValue(Edit*,std::string,bool)), uiproxy, SLOT(onSetEditValue(Edit*,std::string,bool)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setSpinboxValue(Spinbox*,double,bool)), uiproxy, SLOT(onSetSpinboxValue(Spinbox*,double,bool)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setLabelText(Label*,std::string,bool)), uiproxy, SLOT(onSetLabelText(Label*,std::string,bool)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setSliderValue(Slider*,int,bool)), uiproxy, SLOT(onSetSliderValue(Slider*,int,bool)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setCheckboxValue(Checkbox*,int,bool)), uiproxy, SLOT(onSetCheckboxValue(Checkbox*,int,bool)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setRadiobuttonValue(Radiobutton*,int,bool)), uiproxy, SLOT(onSetRadiobuttonValue(Radiobutton*,int,bool)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(insertComboboxItem(Combobox*,int,std::string,bool)), uiproxy, SLOT(onInsertComboboxItem(Combobox*,int,std::string,bool)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(removeComboboxItem(Combobox*,int,bool)), uiproxy, SLOT(onRemoveComboboxItem(Combobox*,int,bool)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setComboboxItems(Combobox*,QStringList&,int,bool)), uiproxy, SLOT(onSetComboboxItems(Combobox*,QStringList&,int,bool)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setComboboxSelectedIndex(Combobox*,int,bool)), uiproxy, SLOT(onSetComboboxSelectedIndex(Combobox*,int,bool)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setCurrentTab(Tabs*,int,bool)), uiproxy, SLOT(onSetCurrentTab(Tabs*,int,bool)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setWidgetVisibility(Widget*,bool)), uiproxy, SLOT(onSetWidgetVisibility(Widget*,bool)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(replot(Plot*)), uiproxy, SLOT(onReplot(Plot*)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(addCurve(Plot*,int,std::string,std::vector<int>,int,curve_options*)), uiproxy, SLOT(onAddCurve(Plot*,int,std::string,std::vector<int>,int,curve_options*)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(addCurveTimePoints(Plot*,std::string,std::vector<double>,std::vector<double>)), uiproxy, SLOT(onAddCurveTimePoints(Plot*,std::string,std::vector<double>,std::vector<double>)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(addCurveXYPoints(Plot*,std::string,std::vector<double>,std::vector<double>,std::vector<double>)), uiproxy, SLOT(onAddCurveXYPoints(Plot*,std::string,std::vector<double>,std::vector<double>,std::vector<double>)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(clearCurve(Plot*,std::string)), uiproxy, SLOT(onClearCurve(Plot*,std::string)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(removeCurve(Plot*,std::string)), uiproxy, SLOT(onRemoveCurve(Plot*,std::string)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setPlotRanges(Plot*,double,double,double,double)), uiproxy, SLOT(onSetPlotRanges(Plot*,double,double,double,double)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setPlotXRange(Plot*,double,double)), uiproxy, SLOT(onSetPlotXRange(Plot*,double,double)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setPlotYRange(Plot*,double,double)), uiproxy, SLOT(onSetPlotYRange(Plot*,double,double)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(growPlotRanges(Plot*,double,double,double,double)), uiproxy, SLOT(onGrowPlotRanges(Plot*,double,double,double,double)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(growPlotXRange(Plot*,double,double)), uiproxy, SLOT(onGrowPlotXRange(Plot*,double,double)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(growPlotYRange(Plot*,double,double)), uiproxy, SLOT(onGrowPlotYRange(Plot*,double,double)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setPlotLabels(Plot*,std::string,std::string)), uiproxy, SLOT(onSetPlotLabels(Plot*,std::string,std::string)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setPlotXLabel(Plot*,std::string)), uiproxy, SLOT(onSetPlotXLabel(Plot*,std::string)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setPlotYLabel(Plot*,std::string)), uiproxy, SLOT(onSetPlotYLabel(Plot*,std::string)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(rescaleAxes(Plot*,std::string,bool,bool)), uiproxy, SLOT(onRescaleAxes(Plot*,std::string,bool,bool)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(rescaleAxesAll(Plot*,bool,bool)), uiproxy, SLOT(onRescaleAxesAll(Plot*,bool,bool)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setMouseOptions(Plot*,bool,bool,bool,bool)), uiproxy, SLOT(onSetMouseOptions(Plot*,bool,bool,bool,bool)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setLegendVisibility(Plot*,bool)), uiproxy, SLOT(onSetLegendVisibility(Plot*,bool)), Qt::BlockingQueuedConnection);
    connect(uiproxy, SIGNAL(plottableClick(Plot*,std::string,int,double,double)), this, SLOT(onPlottableClick(Plot*,std::string,int,double,double)));
    connect(uiproxy, SIGNAL(legendClick(Plot*,std::string)), this, SLOT(onLegendClick(Plot*,std::string)));
}

/**
 * while events are delivered, objects may be deleted in the other thread.
 * (this can happen when stopping the simulation for instance).
 * in order to prevent a crash, we check with Widget/Window::exists(..) if the
 * pointer refers to an object which is still valid (i.e. not deleted)
 */
#define CHECK_POINTER(clazz,p) \
    if(!p) return; \
    if(!clazz::exists(p)) {DBG << "warning: " #clazz << p << " has already been deleted (or the pointer is invalid)" << std::endl; return;} \
    if(!p->proxy) return;

void UIFunctions::onButtonClick(Widget *widget)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, widget);

    EventOnClick *e = dynamic_cast<EventOnClick*>(widget);

    if(!e) return;
    if(e->onclick == "" || widget->proxy->scriptID == -1) return;

    onclickCallback_in in_args;
    in_args.handle = widget->proxy->handle;
    in_args.id = widget->id;
    onclickCallback_out out_args;
    onclickCallback(widget->proxy->scriptID, e->onclick.c_str(), &in_args, &out_args);
}

void UIFunctions::onValueChange(Widget *widget, int value)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, widget);

    /* XXX: spinbox inherits EventOnChangeDouble; however, when float="false",
     *      it emits a valueChanged(int) signal. So we try to read here also
     *      for a EventOnChangeDouble handler here to cover for that case.
     */
    EventOnChangeInt *ei = dynamic_cast<EventOnChangeInt*>(widget);
    EventOnChangeDouble *ed = dynamic_cast<Spinbox*>(widget) ? dynamic_cast<EventOnChangeDouble*>(widget) : 0;

    if(!ei && !ed) return;
    std::string onchange = ei ? ei->onchange : ed->onchange;
    if(onchange == "" || widget->proxy->scriptID == -1) return;

    onchangeIntCallback_in in_args;
    in_args.handle = widget->proxy->handle;
    in_args.id = widget->id;
    in_args.value = value;
    onchangeIntCallback_out out_args;
    onchangeIntCallback(widget->proxy->scriptID, onchange.c_str(), &in_args, &out_args);
}

void UIFunctions::onValueChange(Widget *widget, double value)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, widget);

    EventOnChangeDouble *e = dynamic_cast<EventOnChangeDouble*>(widget);

    if(!e) return;
    if(e->onchange == "" || widget->proxy->scriptID == -1) return;

    onchangeDoubleCallback_in in_args;
    in_args.handle = widget->proxy->handle;
    in_args.id = widget->id;
    in_args.value = value;
    onchangeDoubleCallback_out out_args;
    onchangeDoubleCallback(widget->proxy->scriptID, e->onchange.c_str(), &in_args, &out_args);
}

void UIFunctions::onValueChange(Widget *widget, QString value)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, widget);

    EventOnChangeString *e = dynamic_cast<EventOnChangeString*>(widget);

    if(!e) return;
    if(e->onchange == "" || widget->proxy->scriptID == -1) return;

    onchangeStringCallback_in in_args;
    in_args.handle = widget->proxy->handle;
    in_args.id = widget->id;
    in_args.value = value.toStdString();
    onchangeStringCallback_out out_args;
    onchangeStringCallback(widget->proxy->scriptID, e->onchange.c_str(), &in_args, &out_args);
}

void UIFunctions::onEditingFinished(Edit *edit, QString value)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, edit);

    EventOnEditingFinished *e = dynamic_cast<EventOnEditingFinished*>(edit);

    if(!e) return;
    if(e->oneditingfinished == "" || edit->proxy->scriptID == -1) return;

    oneditingfinishedCallback_in in_args;
    in_args.handle = edit->proxy->handle;
    in_args.id = edit->id;
    in_args.value = value.toStdString();
    oneditingfinishedCallback_out out_args;
    oneditingfinishedCallback(edit->proxy->scriptID, e->oneditingfinished.c_str(), &in_args, &out_args);
}

void UIFunctions::onWindowClose(Window *window)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Window, window);

    oncloseCallback_in in_args;
    in_args.handle = window->proxy->getHandle();
    oncloseCallback_out out_args;
    oncloseCallback(window->proxy->getScriptID(), window->onclose.c_str(), &in_args, &out_args);
}

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
        simReleaseBufferE((simChar *)data);
        setImage(image, (simChar *)scaled, w, h);
    }
    else
    {
        setImage(image, (simChar *)data, resolution[0], resolution[1]);
    }
}

void UIFunctions::onPlottableClick(Plot *plot, std::string name, int index, double x, double y)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, plot);

    if(plot->onCurveClick == "" || plot->proxy->scriptID == -1) return;

    onPlottableClickCallback(plot->proxy->getScriptID(), plot->onCurveClick.c_str(), plot->proxy->getHandle(), plot->id, name, index, x, y);
}

void UIFunctions::onLegendClick(Plot *plot, std::string name)
{
    ASSERT_THREAD(!UI);
    CHECK_POINTER(Widget, plot);

    if(plot->onLegendClick == "" || plot->proxy->scriptID == -1) return;

    onLegendClickCallback(plot->proxy->getScriptID(), plot->onLegendClick.c_str(), plot->proxy->getHandle(), plot->id, name);
}

