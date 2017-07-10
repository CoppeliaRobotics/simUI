// Copyright 2016 Coppelia Robotics GmbH. All rights reserved. 
// marc@coppeliarobotics.com
// www.coppeliarobotics.com
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// -------------------------------------------------------------------
// Authors:
// Federico Ferri <federico.ferri.it at gmail dot com>
// -------------------------------------------------------------------

#include "v_repExtCustomUI.h"
#include "plugin.h"
#include "debug.h"
#include "config.h"
#include "v_repLib.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

#ifdef ENABLE_SIGNAL_SPY
#include "signal_spy.h"
#endif

#include <boost/algorithm/string/predicate.hpp>
#include <boost/foreach.hpp>

#include <QVector>
#include <QThread>
#include <QSlider>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QWidget>
#include <QLabel>
#include <QGroupBox>
#include <QComboBox>
#include <QDialog>

#ifdef _WIN32
    #ifdef QT_COMPIL
        #include <direct.h>
    #else
        #include <shlwapi.h>
        #pragma comment(lib, "Shlwapi.lib")
    #endif
#endif /* _WIN32 */
#if defined (__linux) || defined (__APPLE__)
    #include <unistd.h>
#define _stricmp strcasecmp
#endif /* __linux || __APPLE__ */

#define CONCAT(x, y, z) x y z
#define strConCat(x, y, z)    CONCAT(x, y, z)

LIBRARY vrepLib; // the V-REP library that we will dynamically load and bind

#include "tinyxml2.h"

#include "stubs.h"
#include "Proxy.h"
#include "UIFunctions.h"
#include "UIProxy.h"
#include "widgets/all.h"

void create(SScriptCallBack *p, const char *cmd, create_in *in, create_out *out)
{
    ASSERT_THREAD(!UI);
    DBG << "[enter]" << std::endl;
    tinyxml2::XMLDocument xmldoc;
    tinyxml2::XMLError error = xmldoc.Parse(in->xml.c_str(), in->xml.size());

    if(error != tinyxml2::XML_NO_ERROR)
    {
        simSetLastError(cmd, "XML parse error");
        return;
    }

    tinyxml2::XMLElement *rootElement = xmldoc.FirstChildElement();
    std::map<int, Widget*> widgets;
    Window *window = new Window;
    try
    {
        window->parse(widgets, rootElement);
    }
    catch(std::exception& ex)
    {
        simSetLastError(cmd, ex.what());
        delete window;
        return;
    }

    // determine wether the Proxy object should be destroyed at simulation end
    bool destroy = false;
    int scriptProperty;
    int objectHandle;
    simGetScriptProperty(p->scriptID, &scriptProperty, &objectHandle);
    int scriptType = (scriptProperty | sim_scripttype_threaded) - sim_scripttype_threaded;
    if(scriptType == sim_scripttype_mainscript || scriptType == sim_scripttype_childscript || scriptType == sim_scripttype_jointctrlcallback)
        destroy = true;

    int sceneID = simGetInt32ParameterE(sim_intparam_scene_unique_id);
    DBG << "Creating a new Proxy object... (destroy at simulation end = " << (destroy ? "true" : "false") << ")" << std::endl;
    Proxy *proxy = new Proxy(destroy, sceneID, p->scriptID, window, widgets);
    out->uiHandle = proxy->getHandle();
    DBG << "Proxy " << proxy->getHandle() << " created in scene " << sceneID << std::endl;

    DBG << "call UIFunctions::create() (will emit the create(Proxy*) signal)..." << std::endl;
    UIFunctions::getInstance()->create(proxy); // connected to UIProxy, which
                            // will run code for creating Qt widgets in the UI thread
    DBG << "[leave]" << std::endl;
}

void destroy(SScriptCallBack *p, const char *cmd, destroy_in *in, destroy_out *out)
{
    ASSERT_THREAD(!UI);
    DBG << "[enter]" << std::endl;

    Proxy *proxy = Proxy::byHandle(in->handle);
    if(!proxy)
    {
        DBG << "invalid ui handle: " << in->handle << std::endl;

        simSetLastError(cmd, "invalid ui handle");
        return;
    }

    DBG << "call UIFunctions::destroy() (will emit the destroy(Proxy*) signal)..." << std::endl;
    UIFunctions::getInstance()->destroy(proxy); // will also delete proxy

    DBG << "[leave]" << std::endl;
}

Widget* getWidget(int handle, int id)
{
    Widget *widget = Widget::byId(handle, id);
    if(!widget)
    {
        std::stringstream ss;
        ss << "invalid widget id: " << id;
        throw std::runtime_error(ss.str());
    }
    return widget;
}

template<typename T>
T* getWidget(int handle, int id, const char *cmd, const char *widget_type_name)
{
    T *twidget = dynamic_cast<T*>(getWidget(handle, id));
    if(!twidget)
    {
        std::stringstream ss;
        ss << "invalid widget type. expected " << widget_type_name << " (in function getWidget())";
        throw std::runtime_error(ss.str());
    }
    return twidget;
}

template<typename T>
T* getQWidget(int handle, int id, const char *cmd, const char *widget_type_name)
{
    Widget *widget = getWidget<Widget>(handle, id, cmd, widget_type_name);

    T *qwidget = dynamic_cast<T*>(widget->getQWidget());
    if(!qwidget)
    {
        std::stringstream ss;
        ss << "invalid widget type. expected " << widget_type_name << " (in function getQWidget())";
        throw std::runtime_error(ss.str());
    }

    return qwidget;
}

void setStyleSheet(SScriptCallBack *p, const char *cmd, setStyleSheet_in *in, setStyleSheet_out *out)
{
    ASSERT_THREAD(!UI);
    Widget *widget = getWidget(in->handle, in->id);
    UIFunctions::getInstance()->setStyleSheet(widget, in->styleSheet);
}

void setButtonText(SScriptCallBack *p, const char *cmd, setButtonText_in *in, setButtonText_out *out)
{
    ASSERT_THREAD(!UI);
    Button *button = getWidget<Button>(in->handle, in->id, cmd, "button");
    UIFunctions::getInstance()->setButtonText(button, in->text);
}

void setButtonPressed(SScriptCallBack *p, const char *cmd, setButtonPressed_in *in, setButtonPressed_out *out)
{
    ASSERT_THREAD(!UI);
    Button *button = getWidget<Button>(in->handle, in->id, cmd, "button");
    UIFunctions::getInstance()->setButtonPressed(button, in->pressed);
}

void getSliderValue(SScriptCallBack *p, const char *cmd, getSliderValue_in *in, getSliderValue_out *out)
{
    Slider *slider = getWidget<Slider>(in->handle, in->id, cmd, "slider");
    out->value = slider->getValue();
}

void setSliderValue(SScriptCallBack *p, const char *cmd, setSliderValue_in *in, setSliderValue_out *out)
{
    ASSERT_THREAD(!UI);
    Slider *slider = getWidget<Slider>(in->handle, in->id, cmd, "slider");
    UIFunctions::getInstance()->setSliderValue(slider, in->value, in->suppressEvents);
}

void getEditValue(SScriptCallBack *p, const char *cmd, getEditValue_in *in, getEditValue_out *out)
{
    Edit *edit = getWidget<Edit>(in->handle, in->id, cmd, "edit");
    out->value = edit->getValue();
}

void setEditValue(SScriptCallBack *p, const char *cmd, setEditValue_in *in, setEditValue_out *out)
{
    ASSERT_THREAD(!UI);
    Edit *edit = getWidget<Edit>(in->handle, in->id, cmd, "edit");
    UIFunctions::getInstance()->setEditValue(edit, in->value, in->suppressEvents);
}

void getSpinboxValue(SScriptCallBack *p, const char *cmd, getSpinboxValue_in *in, getSpinboxValue_out *out)
{
    Spinbox *spinbox = getWidget<Spinbox>(in->handle, in->id, cmd, "spinbox");
    out->value = spinbox->getValue();
}

void setSpinboxValue(SScriptCallBack *p, const char *cmd, setSpinboxValue_in *in, setSpinboxValue_out *out)
{
    ASSERT_THREAD(!UI);
    Spinbox *spinbox = getWidget<Spinbox>(in->handle, in->id, cmd, "spinbox");
    UIFunctions::getInstance()->setSpinboxValue(spinbox, in->value, in->suppressEvents);
}

void getCheckboxValue(SScriptCallBack *p, const char *cmd, getCheckboxValue_in *in, getCheckboxValue_out *out)
{
    Checkbox *checkbox = getWidget<Checkbox>(in->handle, in->id, cmd, "checkbox");
    out->value = checkbox->convertValueToInt(checkbox->getValue());
}

void setCheckboxValue(SScriptCallBack *p, const char *cmd, setCheckboxValue_in *in, setCheckboxValue_out *out)
{
    ASSERT_THREAD(!UI);
    Checkbox *checkbox = getWidget<Checkbox>(in->handle, in->id, cmd, "checkbox");
    Qt::CheckState value = checkbox->convertValueFromInt(in->value);
    UIFunctions::getInstance()->setCheckboxValue(checkbox, value, in->suppressEvents);
}

void getRadiobuttonValue(SScriptCallBack *p, const char *cmd, getRadiobuttonValue_in *in, getRadiobuttonValue_out *out)
{
    Radiobutton *radiobutton = getWidget<Radiobutton>(in->handle, in->id, cmd, "radiobutton");
    out->value = radiobutton->convertValueToInt(radiobutton->getValue());
}

void setRadiobuttonValue(SScriptCallBack *p, const char *cmd, setRadiobuttonValue_in *in, setRadiobuttonValue_out *out)
{
    ASSERT_THREAD(!UI);
    Radiobutton *radiobutton = getWidget<Radiobutton>(in->handle, in->id, cmd, "radiobutton");
    bool value = radiobutton->convertValueFromInt(in->value);
    UIFunctions::getInstance()->setRadiobuttonValue(radiobutton, value, in->suppressEvents);
}

void getLabelText(SScriptCallBack *p, const char *cmd, getLabelText_in *in, getLabelText_out *out)
{
    Label *label = getWidget<Label>(in->handle, in->id, cmd, "label");
    out->text = label->getText();
}

void setLabelText(SScriptCallBack *p, const char *cmd, setLabelText_in *in, setLabelText_out *out)
{
    ASSERT_THREAD(!UI);
    Label *label = getWidget<Label>(in->handle, in->id, cmd, "label");
    UIFunctions::getInstance()->setLabelText(label, in->text, in->suppressEvents);
}

void insertComboboxItem(SScriptCallBack *p, const char *cmd, insertComboboxItem_in *in, insertComboboxItem_out *out)
{
    ASSERT_THREAD(!UI);
    Combobox *combobox = getWidget<Combobox>(in->handle, in->id, cmd, "combobox");
    UIFunctions::getInstance()->insertComboboxItem(combobox, in->index, in->text, in->suppressEvents);
}

void removeComboboxItem(SScriptCallBack *p, const char *cmd, removeComboboxItem_in *in, removeComboboxItem_out *out)
{
    ASSERT_THREAD(!UI);
    Combobox *combobox = getWidget<Combobox>(in->handle, in->id, cmd, "combobox");
    UIFunctions::getInstance()->removeComboboxItem(combobox, in->index, in->suppressEvents);
}

void getComboboxItemCount(SScriptCallBack *p, const char *cmd, getComboboxItemCount_in *in, getComboboxItemCount_out *out)
{
    Combobox *combobox = getWidget<Combobox>(in->handle, in->id, cmd, "combobox");
    out->count = combobox->count();
}

void getComboboxItemText(SScriptCallBack *p, const char *cmd, getComboboxItemText_in *in, getComboboxItemText_out *out)
{
    Combobox *combobox = getWidget<Combobox>(in->handle, in->id, cmd, "combobox");
    out->text = combobox->itemText(in->index);
}

void getComboboxItems(SScriptCallBack *p, const char *cmd, getComboboxItems_in *in, getComboboxItems_out *out)
{
    Combobox *combobox = getWidget<Combobox>(in->handle, in->id, cmd, "combobox");
    out->items = combobox->getItems();
}

void setComboboxItems(SScriptCallBack *p, const char *cmd, setComboboxItems_in *in, setComboboxItems_out *out)
{
    ASSERT_THREAD(!UI);
    Combobox *combobox = getWidget<Combobox>(in->handle, in->id, cmd, "combobox");
    UIFunctions::getInstance()->setComboboxItems(combobox, in->items, in->index, in->suppressEvents);
}

void setComboboxSelectedIndex(SScriptCallBack *p, const char *cmd, setComboboxSelectedIndex_in *in, setComboboxSelectedIndex_out *out)
{
    ASSERT_THREAD(!UI);
    Combobox *combobox = getWidget<Combobox>(in->handle, in->id, cmd, "combobox");
    UIFunctions::getInstance()->setComboboxSelectedIndex(combobox, in->index, in->suppressEvents);
}

void hide(SScriptCallBack *p, const char *cmd, hide_in *in, hide_out *out)
{
    ASSERT_THREAD(!UI);
    Proxy *proxy = Proxy::byHandle(in->handle);
    if(!proxy)
    {
        simSetLastError(cmd, "invalid ui handle");
        return;
    }

    UIFunctions::getInstance()->hideWindow(proxy->getWidget());
}

void show(SScriptCallBack *p, const char *cmd, show_in *in, show_out *out)
{
    ASSERT_THREAD(!UI);
    Proxy *proxy = Proxy::byHandle(in->handle);
    if(!proxy)
    {
        simSetLastError(cmd, "invalid ui handle");
        return;
    }

    UIFunctions::getInstance()->showWindow(proxy->getWidget());
}

void isVisible(SScriptCallBack *p, const char *cmd, isVisible_in *in, isVisible_out *out)
{
    Proxy *proxy = Proxy::byHandle(in->handle);
    if(!proxy)
    {
        simSetLastError(cmd, "invalid ui handle");
        return;
    }

    out->visibility = proxy->getWidget()->getQWidget()->isVisible();
}

void getPosition(SScriptCallBack *p, const char *cmd, getPosition_in *in, getPosition_out *out)
{
    Proxy *proxy = Proxy::byHandle(in->handle);
    if(!proxy)
    {
        simSetLastError(cmd, "invalid ui handle");
        return;
    }

    QWidget *window = proxy->getWidget()->getQWidget();
    out->x = window->x();
    out->y = window->y();
}

void setPosition(SScriptCallBack *p, const char *cmd, setPosition_in *in, setPosition_out *out)
{
    ASSERT_THREAD(!UI);
    Proxy *proxy = Proxy::byHandle(in->handle);
    if(!proxy)
    {
        simSetLastError(cmd, "invalid ui handle");
        return;
    }

    Window *window = proxy->getWidget();
    UIFunctions::getInstance()->setPosition(window, in->x, in->y);
}

void getSize(SScriptCallBack *p, const char *cmd, getSize_in *in, getSize_out *out)
{
    Proxy *proxy = Proxy::byHandle(in->handle);
    if(!proxy)
    {
        simSetLastError(cmd, "invalid ui handle");
        return;
    }

    QWidget *window = proxy->getWidget()->getQWidget();
    out->w = window->width();
    out->h = window->height();
}

void setSize(SScriptCallBack *p, const char *cmd, setSize_in *in, setSize_out *out)
{
    ASSERT_THREAD(!UI);
    Proxy *proxy = Proxy::byHandle(in->handle);
    if(!proxy)
    {
        simSetLastError(cmd, "invalid ui handle");
        return;
    }

    Window *window = proxy->getWidget();
    UIFunctions::getInstance()->setSize(window, in->w, in->h);
}

void getTitle(SScriptCallBack *p, const char *cmd, getTitle_in *in, getTitle_out *out)
{
    Proxy *proxy = Proxy::byHandle(in->handle);
    if(!proxy)
    {
        simSetLastError(cmd, "invalid ui handle");
        return;
    }

    QWidget *window = proxy->getWidget()->getQWidget();
    out->title = static_cast<QDialog*>(window)->windowTitle().toStdString();
}

void setTitle(SScriptCallBack *p, const char *cmd, setTitle_in *in, setTitle_out *out)
{
    ASSERT_THREAD(!UI);
    Proxy *proxy = Proxy::byHandle(in->handle);
    if(!proxy)
    {
        simSetLastError(cmd, "invalid ui handle");
        return;
    }

    Window *window = proxy->getWidget();
    UIFunctions::getInstance()->setTitle(window, in->title);
}

void setImageData(SScriptCallBack *p, const char *cmd, setImageData_in *in, setImageData_out *out)
{
    ASSERT_THREAD(!UI);
    Image *imageWidget = dynamic_cast<Image*>(Widget::byId(in->handle, in->id));
    if(!imageWidget)
    {
        std::stringstream ss;
        ss << "invalid image widget id: " << in->id;
        throw std::runtime_error(ss.str());
    }

    int bpp = 3; // bytes per pixel
    int sz = in->width * in->height * bpp;
    if(in->data.size() != sz)
    {
        std::stringstream ss;
        ss << "bad image size. expected: " << sz << ". got: " << in->data.size() << ".";
        throw std::runtime_error(ss.str());
    }

    simChar *img = simCreateBufferE(sz);
    std::memcpy(img, in->data.c_str(), sz);
    simInt resolution[2] = {in->width, in->height};
    simTransformImage((simUChar *)img, resolution, 4, NULL, NULL, NULL);

    UIFunctions::getInstance()->setImage(imageWidget, img, in->width, in->height);
}

void setEnabled(SScriptCallBack *p, const char *cmd, setEnabled_in *in, setEnabled_out *out)
{
    ASSERT_THREAD(!UI);
    Widget *widget = Widget::byId(in->handle, in->id);
    if(!widget)
    {
        std::stringstream ss;
        ss << "invalid widget id: " << in->id;
        throw std::runtime_error(ss.str());
    }

    UIFunctions::getInstance()->setEnabled(widget, in->enabled);
}

void getCurrentTab(SScriptCallBack *p, const char *cmd, getCurrentTab_in *in, getCurrentTab_out *out)
{
    QTabWidget *tabs = getQWidget<QTabWidget>(in->handle, in->id, cmd, "tabs");
    out->index = tabs->currentIndex();
}

void setCurrentTab(SScriptCallBack *p, const char *cmd, setCurrentTab_in *in, setCurrentTab_out *out)
{
    ASSERT_THREAD(!UI);
    Tabs *tabs = getWidget<Tabs>(in->handle, in->id, cmd, "tabs");
    UIFunctions::getInstance()->setCurrentTab(tabs, in->index, in->suppressEvents);
}

void getWidgetVisibility(SScriptCallBack *p, const char *cmd, getWidgetVisibility_in *in, getWidgetVisibility_out *out)
{
    QWidget *widget = getQWidget<QWidget>(in->handle, in->id, cmd, "widget");
    out->visibility = widget->isVisible();
}

void setWidgetVisibility(SScriptCallBack *p, const char *cmd, setWidgetVisibility_in *in, setWidgetVisibility_out *out)
{
    ASSERT_THREAD(!UI);
    Widget *widget = Widget::byId(in->handle, in->id);
    if(!widget)
    {
        std::stringstream ss;
        ss << "invalid widget id: " << in->id;
        throw std::runtime_error(ss.str());
    }

    UIFunctions::getInstance()->setWidgetVisibility(widget, in->visibility);
}

void getCurrentEditWidget(SScriptCallBack *p, const char *cmd, getCurrentEditWidget_in *in, getCurrentEditWidget_out *out)
{
    Proxy *proxy = Proxy::byHandle(in->handle);
    if(!proxy)
    {
        simSetLastError(cmd, "invalid ui handle");
        return;
    }
    QWidget *window = proxy->getWidget()->getQWidget();
    QLineEdit* qedit = NULL;
    QList<QLineEdit*> wl = window->findChildren<QLineEdit*>(QString());
    for(int i = 0; i < wl.size(); i++)
    {
        if(wl[i]->selectedText().size() > 0)
        {
            qedit = wl[i];
            break;
        }
    }
    Widget *edit = Widget::byQWidget(qedit);
    if(edit)
        out->id = edit->getId();
}

void setCurrentEditWidget(SScriptCallBack *p, const char *cmd, setCurrentEditWidget_in *in, setCurrentEditWidget_out *out)
{
    if(in->id == -1) return;
    Edit *edit = getWidget<Edit>(in->handle, in->id, cmd, "edit");
    QLineEdit *qedit = static_cast<QLineEdit*>(edit->getQWidget());
    qedit->setFocus();
    qedit->selectAll();
}

void replot(SScriptCallBack *p, const char *cmd, replot_in *in, replot_out *out)
{
    Plot *plot = getWidget<Plot>(in->handle, in->id, cmd, "plot");
    UIFunctions::getInstance()->replot(plot);
}

void addCurve(SScriptCallBack *p, const char *cmd, addCurve_in *in, addCurve_out *out)
{
    Plot *plot = getWidget<Plot>(in->handle, in->id, cmd, "plot");
    plot->curveNameMustNotExist(in->name);
    UIFunctions::getInstance()->addCurve(plot, in->type, in->name, in->color, in->style, &in->options);
}

void addCurveTimePoints(SScriptCallBack *p, const char *cmd, addCurveTimePoints_in *in, addCurveTimePoints_out *out)
{
    Plot *plot = getWidget<Plot>(in->handle, in->id, cmd, "plot");
    QCPAbstractPlottable *curve = plot->curveNameMustExist(in->name)->second;
    plot->curveMustBeTime(curve);
    UIFunctions::getInstance()->addCurveTimePoints(plot, in->name, in->x, in->y);
}

void addCurveXYPoints(SScriptCallBack *p, const char *cmd, addCurveXYPoints_in *in, addCurveXYPoints_out *out)
{
    Plot *plot = getWidget<Plot>(in->handle, in->id, cmd, "plot");
    QCPAbstractPlottable *curve = plot->curveNameMustExist(in->name)->second;
    plot->curveMustBeXY(curve);
    UIFunctions::getInstance()->addCurveXYPoints(plot, in->name, in->t, in->x, in->y);
}

void clearCurve(SScriptCallBack *p, const char *cmd, clearCurve_in *in, clearCurve_out *out)
{
    Plot *plot = getWidget<Plot>(in->handle, in->id, cmd, "plot");
    plot->curveNameMustExist(in->name);
    UIFunctions::getInstance()->clearCurve(plot, in->name);
}

void removeCurve(SScriptCallBack *p, const char *cmd, removeCurve_in *in, removeCurve_out *out)
{
    Plot *plot = getWidget<Plot>(in->handle, in->id, cmd, "plot");
    plot->curveNameMustExist(in->name);
    UIFunctions::getInstance()->removeCurve(plot, in->name);
}

void setPlotRanges(SScriptCallBack *p, const char *cmd, setPlotRanges_in *in, setPlotRanges_out *out)
{
    Plot *plot = getWidget<Plot>(in->handle, in->id, cmd, "plot");
    UIFunctions::getInstance()->setPlotRanges(plot, in->xmin, in->xmax, in->ymin, in->ymax);
}

void setPlotXRange(SScriptCallBack *p, const char *cmd, setPlotXRange_in *in, setPlotXRange_out *out)
{
    Plot *plot = getWidget<Plot>(in->handle, in->id, cmd, "plot");
    UIFunctions::getInstance()->setPlotXRange(plot, in->xmin, in->xmax);
}

void setPlotYRange(SScriptCallBack *p, const char *cmd, setPlotYRange_in *in, setPlotYRange_out *out)
{
    Plot *plot = getWidget<Plot>(in->handle, in->id, cmd, "plot");
    UIFunctions::getInstance()->setPlotYRange(plot, in->ymin, in->ymax);
}

void growPlotRanges(SScriptCallBack *p, const char *cmd, growPlotRanges_in *in, growPlotRanges_out *out)
{
    Plot *plot = getWidget<Plot>(in->handle, in->id, cmd, "plot");
    UIFunctions::getInstance()->growPlotRanges(plot, in->xmin, in->xmax, in->ymin, in->ymax);
}

void growPlotXRange(SScriptCallBack *p, const char *cmd, growPlotXRange_in *in, growPlotXRange_out *out)
{
    Plot *plot = getWidget<Plot>(in->handle, in->id, cmd, "plot");
    UIFunctions::getInstance()->growPlotXRange(plot, in->xmin, in->xmax);
}

void growPlotYRange(SScriptCallBack *p, const char *cmd, growPlotYRange_in *in, growPlotYRange_out *out)
{
    Plot *plot = getWidget<Plot>(in->handle, in->id, cmd, "plot");
    UIFunctions::getInstance()->growPlotYRange(plot, in->ymin, in->ymax);
}

void setPlotLabels(SScriptCallBack *p, const char *cmd, setPlotLabels_in *in, setPlotLabels_out *out)
{
    Plot *plot = getWidget<Plot>(in->handle, in->id, cmd, "plot");
    UIFunctions::getInstance()->setPlotLabels(plot, in->x, in->y);
}

void setPlotXLabel(SScriptCallBack *p, const char *cmd, setPlotXLabel_in *in, setPlotXLabel_out *out)
{
    Plot *plot = getWidget<Plot>(in->handle, in->id, cmd, "plot");
    UIFunctions::getInstance()->setPlotXLabel(plot, in->label);
}

void setPlotYLabel(SScriptCallBack *p, const char *cmd, setPlotYLabel_in *in, setPlotYLabel_out *out)
{
    Plot *plot = getWidget<Plot>(in->handle, in->id, cmd, "plot");
    UIFunctions::getInstance()->setPlotYLabel(plot, in->label);
}

void rescaleAxes(SScriptCallBack *p, const char *cmd, rescaleAxes_in *in, rescaleAxes_out *out)
{
    Plot *plot = getWidget<Plot>(in->handle, in->id, cmd, "plot");
    plot->curveNameMustExist(in->name);
    UIFunctions::getInstance()->rescaleAxes(plot, in->name, in->onlyEnlargeX, in->onlyEnlargeY);
}

void rescaleAxesAll(SScriptCallBack *p, const char *cmd, rescaleAxesAll_in *in, rescaleAxesAll_out *out)
{
    Plot *plot = getWidget<Plot>(in->handle, in->id, cmd, "plot");
    UIFunctions::getInstance()->rescaleAxesAll(plot, in->onlyEnlargeX, in->onlyEnlargeY);
}

void setMouseOptions(SScriptCallBack *p, const char *cmd, setMouseOptions_in *in, setMouseOptions_out *out)
{
    Plot *plot = getWidget<Plot>(in->handle, in->id, cmd, "plot");
    UIFunctions::getInstance()->setMouseOptions(plot, in->panX, in->panY, in->zoomX, in->zoomY);
}

void setLegendVisibility(SScriptCallBack *p, const char *cmd, setLegendVisibility_in *in, setLegendVisibility_out *out)
{
    Plot *plot = getWidget<Plot>(in->handle, in->id, cmd, "plot");
    UIFunctions::getInstance()->setLegendVisibility(plot, in->visible);
}

void getCurveData(SScriptCallBack *p, const char *cmd, getCurveData_in *in, getCurveData_out *out)
{
    Plot *plot = getWidget<Plot>(in->handle, in->id, cmd, "plot");
    plot->getCurveData(in->name, out->x, out->x, out->y);
}

void clearTable(SScriptCallBack *p, const char *cmd, clearTable_in *in, clearTable_out *out)
{
    Table *table = getWidget<Table>(in->handle, in->id, cmd, "table");
    UIFunctions::getInstance()->clearTable(table, in->suppressEvents);
}

void setRowCount(SScriptCallBack *p, const char *cmd, setRowCount_in *in, setRowCount_out *out)
{
    Table *table = getWidget<Table>(in->handle, in->id, cmd, "table");
    UIFunctions::getInstance()->setRowCount(table, in->count, in->suppressEvents);
}

void setColumnCount(SScriptCallBack *p, const char *cmd, setColumnCount_in *in, setColumnCount_out *out)
{
    Widget *widget = getWidget(in->handle, in->id);
    if(Table *table = dynamic_cast<Table*>(widget))
        UIFunctions::getInstance()->setColumnCountTable(table, in->count, in->suppressEvents);
    else if(Tree *tree = dynamic_cast<Tree*>(widget))
        UIFunctions::getInstance()->setColumnCountTree(tree, in->count, in->suppressEvents);
    else
        throw std::runtime_error("invalid widget type. expected table or tree.");
}

void setItem(SScriptCallBack *p, const char *cmd, setItem_in *in, setItem_out *out)
{
    Table *table = getWidget<Table>(in->handle, in->id, cmd, "table");
    UIFunctions::getInstance()->setItem(table, in->row, in->column, in->text, in->suppressEvents);
}

void getRowCount(SScriptCallBack *p, const char *cmd, getRowCount_in *in, getRowCount_out *out)
{
    Table *table = getWidget<Table>(in->handle, in->id, cmd, "table");
    out->count = table->getRowCount();
}

void getColumnCount(SScriptCallBack *p, const char *cmd, getColumnCount_in *in, getColumnCount_out *out)
{
    Widget *widget = getWidget(in->handle, in->id);
    if(Table *table = dynamic_cast<Table*>(widget))
        out->count = table->getColumnCount();
    else if(Tree *tree = dynamic_cast<Tree*>(widget))
        out->count = tree->getColumnCount();
    else
        throw std::runtime_error("invalid widget type. expected table or tree.");
}

void getItem(SScriptCallBack *p, const char *cmd, getItem_in *in, getItem_out *out)
{
    Table *table = getWidget<Table>(in->handle, in->id, cmd, "table");
    out->text = table->getItem(in->row, in->column);
}

void setRowHeaderText(SScriptCallBack *p, const char *cmd, setRowHeaderText_in *in, setRowHeaderText_out *out)
{
    Table *table = getWidget<Table>(in->handle, in->id, cmd, "table");
    UIFunctions::getInstance()->setRowHeaderText(table, in->row, in->text);
}

void setColumnHeaderText(SScriptCallBack *p, const char *cmd, setColumnHeaderText_in *in, setColumnHeaderText_out *out)
{
    Widget *widget = getWidget(in->handle, in->id);
    if(Table *table = dynamic_cast<Table*>(widget))
        UIFunctions::getInstance()->setColumnHeaderTextTable(table, in->column, in->text);
    else if(Tree *tree = dynamic_cast<Tree*>(widget))
        UIFunctions::getInstance()->setColumnHeaderTextTree(tree, in->column, in->text);
    else
        throw std::runtime_error("invalid widget type. expected table or tree.");
}

void setItemEditable(SScriptCallBack *p, const char *cmd, setItemEditable_in *in, setItemEditable_out *out)
{
    Table *table = getWidget<Table>(in->handle, in->id, cmd, "table");
    UIFunctions::getInstance()->setItemEditable(table, in->row, in->column, in->editable);
}

void saveState(SScriptCallBack *p, const char *cmd, saveState_in *in, saveState_out *out)
{
    Widget *widget = getWidget(in->handle, in->id);
    if(Table *table = dynamic_cast<Table*>(widget))
        out->state = table->saveState();
    else if(Tree *tree = dynamic_cast<Tree*>(widget))
        out->state = tree->saveState();
    else
        throw std::runtime_error("invalid widget type. expected table or tree.");
}

void restoreState(SScriptCallBack *p, const char *cmd, restoreState_in *in, restoreState_out *out)
{
    Widget *widget = getWidget(in->handle, in->id);
    if(Table *table = dynamic_cast<Table*>(widget))
        UIFunctions::getInstance()->restoreStateTable(table, in->state);
    else if(Tree *tree = dynamic_cast<Tree*>(widget))
        UIFunctions::getInstance()->restoreStateTree(tree, in->state);
    else
        throw std::runtime_error("invalid widget type. expected table or tree.");
}

void setRowHeight(SScriptCallBack *p, const char *cmd, setRowHeight_in *in, setRowHeight_out *out)
{
    Table *table = getWidget<Table>(in->handle, in->id, cmd, "table");
    UIFunctions::getInstance()->setRowHeight(table, in->row, in->min_size, in->max_size);
}

void setColumnWidth(SScriptCallBack *p, const char *cmd, setColumnWidth_in *in, setColumnWidth_out *out)
{
    Widget *widget = getWidget(in->handle, in->id);
    if(Table *table = dynamic_cast<Table*>(widget))
        UIFunctions::getInstance()->setColumnWidthTable(table, in->column, in->min_size, in->max_size);
    else if(Tree *tree = dynamic_cast<Tree*>(widget))
        UIFunctions::getInstance()->setColumnWidthTree(tree, in->column, in->min_size, in->max_size);
    else
        throw std::runtime_error("invalid widget type. expected table or tree.");
}

void setTableSelection(SScriptCallBack *p, const char *cmd, setTableSelection_in *in, setTableSelection_out *out)
{
    Table *table = getWidget<Table>(in->handle, in->id, cmd, "table");
    UIFunctions::getInstance()->setTableSelection(table, in->row, in->column, in->suppressEvents);
}

void setProgress(SScriptCallBack *p, const char *cmd, setProgress_in *in, setProgress_out *out)
{
    Progressbar *progressbar = getWidget<Progressbar>(in->handle, in->id, cmd, "progressbar");
    UIFunctions::getInstance()->setProgress(progressbar, in->value);
}

void clearTree(SScriptCallBack *p, const char *cmd, clearTree_in *in, clearTree_out *out)
{
    Tree *tree = getWidget<Tree>(in->handle, in->id, cmd, "tree");
    UIFunctions::getInstance()->clearTree(tree, in->suppressEvents);
}

void addTreeItem(SScriptCallBack *p, const char *cmd, addTreeItem_in *in, addTreeItem_out *out)
{
    Tree *tree = getWidget<Tree>(in->handle, in->id, cmd, "tree");
    UIFunctions::getInstance()->addTreeItem(tree, in->item_id, in->parent_id, in->text, in->expanded, in->suppressEvents);
}

void updateTreeItemText(SScriptCallBack *p, const char *cmd, updateTreeItemText_in *in, updateTreeItemText_out *out)
{
    Tree *tree = getWidget<Tree>(in->handle, in->id, cmd, "tree");
    UIFunctions::getInstance()->updateTreeItemText(tree, in->item_id, in->text);
}

void updateTreeItemParent(SScriptCallBack *p, const char *cmd, updateTreeItemParent_in *in, updateTreeItemParent_out *out)
{
    Tree *tree = getWidget<Tree>(in->handle, in->id, cmd, "tree");
    UIFunctions::getInstance()->updateTreeItemParent(tree, in->item_id, in->parent_id, in->suppressEvents);
}

void removeTreeItem(SScriptCallBack *p, const char *cmd, removeTreeItem_in *in, removeTreeItem_out *out)
{
    Tree *tree = getWidget<Tree>(in->handle, in->id, cmd, "tree");
    UIFunctions::getInstance()->removeTreeItem(tree, in->item_id, in->suppressEvents);
}

void setTreeSelection(SScriptCallBack *p, const char *cmd, setTreeSelection_in *in, setTreeSelection_out *out)
{
    Tree *tree = getWidget<Tree>(in->handle, in->id, cmd, "tree");
    UIFunctions::getInstance()->setTreeSelection(tree, in->item_id, in->suppressEvents);
}

void expandAll(SScriptCallBack *p, const char *cmd, expandAll_in *in, expandAll_out *out)
{
    Tree *tree = getWidget<Tree>(in->handle, in->id, cmd, "tree");
    UIFunctions::getInstance()->expandAll(tree, in->suppressEvents);
}

void collapseAll(SScriptCallBack *p, const char *cmd, collapseAll_in *in, collapseAll_out *out)
{
    Tree *tree = getWidget<Tree>(in->handle, in->id, cmd, "tree");
    UIFunctions::getInstance()->collapseAll(tree, in->suppressEvents);
}

void expandToDepth(SScriptCallBack *p, const char *cmd, expandToDepth_in *in, expandToDepth_out *out)
{
    Tree *tree = getWidget<Tree>(in->handle, in->id, cmd, "tree");
    UIFunctions::getInstance()->expandToDepth(tree, in->depth, in->suppressEvents);
}

void addNode(SScriptCallBack *p, const char *cmd, addNode_in *in, addNode_out *out)
{
    Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, cmd, "dataflow");
    out->nodeId = dataflow->nextId();
    UIFunctions::getInstance()->addNode(dataflow, out->nodeId, QPoint(in->x, in->y), QString::fromStdString(in->text), in->inlets, in->outlets);
}

void removeNode(SScriptCallBack *p, const char *cmd, removeNode_in *in, removeNode_out *out)
{
    Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, cmd, "dataflow");
    UIFunctions::getInstance()->removeNode(dataflow, in->nodeId);
}

void setNodeValid(SScriptCallBack *p, const char *cmd, setNodeValid_in *in, setNodeValid_out *out)
{
    Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, cmd, "dataflow");
    dataflow->getNode(in->nodeId);
    UIFunctions::getInstance()->setNodeValid(dataflow, in->nodeId, in->valid);
}

void isNodeValid(SScriptCallBack *p, const char *cmd, isNodeValid_in *in, isNodeValid_out *out)
{
    Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, cmd, "dataflow");
    dataflow->getNode(in->nodeId);
    out->valid = dataflow->isNodeValid(in->nodeId);
}

void setNodePos(SScriptCallBack *p, const char *cmd, setNodePos_in *in, setNodePos_out *out)
{
    Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, cmd, "dataflow");
    dataflow->getNode(in->nodeId);
    UIFunctions::getInstance()->setNodePos(dataflow, in->nodeId, QPoint(in->x, in->y));
}

void getNodePos(SScriptCallBack *p, const char *cmd, getNodePos_in *in, getNodePos_out *out)
{
    Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, cmd, "dataflow");
    dataflow->getNode(in->nodeId);
    QPoint pos = dataflow->getNodePos(in->nodeId);
    out->x = pos.x();
    out->y = pos.y();
}

void setNodeText(SScriptCallBack *p, const char *cmd, setNodeText_in *in, setNodeText_out *out)
{
    Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, cmd, "dataflow");
    dataflow->getNode(in->nodeId);
    UIFunctions::getInstance()->setNodeText(dataflow, in->nodeId, QString::fromStdString(in->text));
}

void getNodeText(SScriptCallBack *p, const char *cmd, getNodeText_in *in, getNodeText_out *out)
{
    Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, cmd, "dataflow");
    dataflow->getNode(in->nodeId);
    out->text = dataflow->getNodeText(in->nodeId);
}

void setNodeInletCount(SScriptCallBack *p, const char *cmd, setNodeInletCount_in *in, setNodeInletCount_out *out)
{
    Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, cmd, "dataflow");
    dataflow->getNode(in->nodeId);
    UIFunctions::getInstance()->setNodeInletCount(dataflow, in->nodeId, in->count);
}

void getNodeInletCount(SScriptCallBack *p, const char *cmd, getNodeInletCount_in *in, getNodeInletCount_out *out)
{
    Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, cmd, "dataflow");
    dataflow->getNode(in->nodeId);
    out->count = dataflow->getNodeInletCount(in->nodeId);
}

void setNodeOutletCount(SScriptCallBack *p, const char *cmd, setNodeOutletCount_in *in, setNodeOutletCount_out *out)
{
    Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, cmd, "dataflow");
    dataflow->getNode(in->nodeId);
    UIFunctions::getInstance()->setNodeOutletCount(dataflow, in->nodeId, in->count);
}

void getNodeOutletCount(SScriptCallBack *p, const char *cmd, getNodeOutletCount_in *in, getNodeOutletCount_out *out)
{
    Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, cmd, "dataflow");
    dataflow->getNode(in->nodeId);
    out->count = dataflow->getNodeOutletCount(in->nodeId);
}

void addConnection(SScriptCallBack *p, const char *cmd, addConnection_in *in, addConnection_out *out)
{
    Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, cmd, "dataflow");
    dataflow->getNode(in->srcNodeId);
    dataflow->getNode(in->dstNodeId);
    UIFunctions::getInstance()->addConnection(dataflow, in->srcNodeId, in->srcOutlet, in->dstNodeId, in->dstInlet);
}

void removeConnection(SScriptCallBack *p, const char *cmd, removeConnection_in *in, removeConnection_out *out)
{
    Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, cmd, "dataflow");
    dataflow->getNode(in->srcNodeId);
    dataflow->getNode(in->dstNodeId);
    UIFunctions::getInstance()->removeConnection(dataflow, in->srcNodeId, in->srcOutlet, in->dstNodeId, in->dstInlet);
}

void setText(SScriptCallBack *p, const char *cmd, setText_in *in, setText_out *out)
{
    TextBrowser *textbrowser = getWidget<TextBrowser>(in->handle, in->id, cmd, "text-browser");
    UIFunctions::getInstance()->setText(textbrowser, in->text, in->suppressEvents);
}

void setUrl(SScriptCallBack *p, const char *cmd, setUrl_in *in, setUrl_out *out)
{
    TextBrowser *textbrowser = getWidget<TextBrowser>(in->handle, in->id, cmd, "text-browser");
    UIFunctions::getInstance()->setUrl(textbrowser, in->url);
}

void addScene3DNode(SScriptCallBack *p, const char *cmd, addScene3DNode_in *in, addScene3DNode_out *out)
{
    Scene3D *scene3d = getWidget<Scene3D>(in->handle, in->id, cmd, "scene3d");
    if(scene3d->nodeExists(in->nodeId)) throw std::runtime_error("node id already exists");
    if(!scene3d->nodeExists(in->parentNodeId)) throw std::runtime_error("parent node id does not exist");
    if(!scene3d->nodeTypeIsValid(in->type)) throw std::runtime_error("invalid node type");
    UIFunctions::getInstance()->addScene3DNode(scene3d, in->nodeId, in->parentNodeId, in->type);
}

void removeScene3DNode(SScriptCallBack *p, const char *cmd, removeScene3DNode_in *in, removeScene3DNode_out *out)
{
    Scene3D *scene3d = getWidget<Scene3D>(in->handle, in->id, cmd, "scene3d");
    if(!scene3d->nodeExists(in->nodeId)) throw std::runtime_error("invalid node id");
    UIFunctions::getInstance()->removeScene3DNode(scene3d, in->nodeId);
}

void setScene3DNodeEnabled(SScriptCallBack *p, const char *cmd, setScene3DNodeEnabled_in *in, setScene3DNodeEnabled_out *out)
{
    Scene3D *scene3d = getWidget<Scene3D>(in->handle, in->id, cmd, "scene3d");
    if(!scene3d->nodeExists(in->nodeId)) throw std::runtime_error("invalid node id");
    UIFunctions::getInstance()->setScene3DNodeEnabled(scene3d, in->nodeId, in->enabled);
}

void setScene3DNodeIntParam(SScriptCallBack *p, const char *cmd, setScene3DNodeIntParam_in *in, setScene3DNodeIntParam_out *out)
{
    Scene3D *scene3d = getWidget<Scene3D>(in->handle, in->id, cmd, "scene3d");
    if(!scene3d->nodeExists(in->nodeId)) throw std::runtime_error("invalid node id");
    UIFunctions::getInstance()->setScene3DIntParam(scene3d, in->nodeId, in->paramName, in->value);
}

void setScene3DNodeFloatParam(SScriptCallBack *p, const char *cmd, setScene3DNodeFloatParam_in *in, setScene3DNodeFloatParam_out *out)
{
    Scene3D *scene3d = getWidget<Scene3D>(in->handle, in->id, cmd, "scene3d");
    if(!scene3d->nodeExists(in->nodeId)) throw std::runtime_error("invalid node id");
    UIFunctions::getInstance()->setScene3DFloatParam(scene3d, in->nodeId, in->paramName, in->value);
}

void setScene3DNodeStringParam(SScriptCallBack *p, const char *cmd, setScene3DNodeStringParam_in *in, setScene3DNodeStringParam_out *out)
{
    Scene3D *scene3d = getWidget<Scene3D>(in->handle, in->id, cmd, "scene3d");
    if(!scene3d->nodeExists(in->nodeId)) throw std::runtime_error("invalid node id");
    UIFunctions::getInstance()->setScene3DStringParam(scene3d, in->nodeId, in->paramName, in->value);
}

void setScene3DNodeVector2Param(SScriptCallBack *p, const char *cmd, setScene3DNodeVector2Param_in *in, setScene3DNodeVector2Param_out *out)
{
    Scene3D *scene3d = getWidget<Scene3D>(in->handle, in->id, cmd, "scene3d");
    if(!scene3d->nodeExists(in->nodeId)) throw std::runtime_error("invalid node id");
    UIFunctions::getInstance()->setScene3DVector2Param(scene3d, in->nodeId, in->paramName, in->x, in->y);
}

void setScene3DNodeVector3Param(SScriptCallBack *p, const char *cmd, setScene3DNodeVector3Param_in *in, setScene3DNodeVector3Param_out *out)
{
    Scene3D *scene3d = getWidget<Scene3D>(in->handle, in->id, cmd, "scene3d");
    if(!scene3d->nodeExists(in->nodeId)) throw std::runtime_error("invalid node id");
    UIFunctions::getInstance()->setScene3DVector3Param(scene3d, in->nodeId, in->paramName, in->x, in->y, in->z);
}

void setScene3DNodeVector4Param(SScriptCallBack *p, const char *cmd, setScene3DNodeVector4Param_in *in, setScene3DNodeVector4Param_out *out)
{
    Scene3D *scene3d = getWidget<Scene3D>(in->handle, in->id, cmd, "scene3d");
    if(!scene3d->nodeExists(in->nodeId)) throw std::runtime_error("invalid node id");
    UIFunctions::getInstance()->setScene3DVector4Param(scene3d, in->nodeId, in->paramName, in->x, in->y, in->z, in->w);
}

VREP_DLLEXPORT unsigned char v_repStart(void* reservedPointer, int reservedInt)
{
    char curDirAndFile[1024];
#ifdef _WIN32
    #ifdef QT_COMPIL
        _getcwd(curDirAndFile, sizeof(curDirAndFile));
    #else
        GetModuleFileNameA(NULL, curDirAndFile, 1023);
        PathRemoveFileSpecA(curDirAndFile);
    #endif
#elif defined (__linux) || defined (__APPLE__)
    getcwd(curDirAndFile, sizeof(curDirAndFile));
#endif

    std::string currentDirAndPath(curDirAndFile);
    std::string temp(currentDirAndPath);
#ifdef _WIN32
    temp+="\\v_rep.dll";
#elif defined (__linux)
    temp+="/libv_rep.so";
#elif defined (__APPLE__)
    temp+="/libv_rep.dylib";
#endif /* __linux || __APPLE__ */
    vrepLib = loadVrepLibrary(temp.c_str());
    if(vrepLib == NULL)
    {
        std::cout << "Error, could not find or correctly load the V-REP library. Cannot start '" PLUGIN_NAME "' plugin.\n";
        return 0;
    }
    if(getVrepProcAddresses(vrepLib)==0)
    {
        std::cout << "Error, could not find all required functions in the V-REP library. Cannot start '" PLUGIN_NAME "' plugin.\n";
        unloadVrepLibrary(vrepLib);
        return 0;
    }

    int vrepVer;
    simGetIntegerParameter(sim_intparam_program_version, &vrepVer);
    if(vrepVer < 30203) // if V-REP version is smaller than 3.02.03
    {
        std::cout << "Sorry, your V-REP copy is somewhat old. Cannot start '" PLUGIN_NAME "' plugin.\n";
        unloadVrepLibrary(vrepLib);
        return 0;
    }

    if(simGetMainWindow(1) == NULL)
    {
        std::cout << "Initialization failed, running in headless mode. Cannot start '" PLUGIN_NAME "' plugin.\n";
        return 0;
    }

    if(!registerScriptStuff())
    {
        std::cout << "Initialization failed.\n";
        unloadVrepLibrary(vrepLib);
        return 0;
    }
#include "lua_calltips.cpp"

#if defined(ENABLE_SIGNAL_SPY) && defined(DEBUG)
    SignalSpy::start();
#endif

    UIProxy::getInstance(); // construct UIProxy here (UI thread)

    return PLUGIN_VERSION; // initialization went fine, we return the version number of this plugin (can be queried with simGetModuleName)
}

VREP_DLLEXPORT void v_repEnd()
{
    ASSERT_THREAD(UI);
    DBG << "[enter]" << std::endl;

    Proxy::destroyAllObjectsFromUIThread();

    UIFunctions::destroyInstance();
    UIProxy::destroyInstance();

    unloadVrepLibrary(vrepLib); // release the library

    DBG << "[leave]" << std::endl;
}

VREP_DLLEXPORT void* v_repMessage(int message, int* auxiliaryData, void* customData, int* replyData)
{
    // Keep following 4 lines at the beginning and unchanged:
    int errorModeSaved;
    simGetIntegerParameter(sim_intparam_error_report_mode, &errorModeSaved);
    simSetIntegerParameter(sim_intparam_error_report_mode, sim_api_errormessage_ignore);
    void* retVal=NULL;

    static bool firstInstancePass = true;
    if(firstInstancePass && message == sim_message_eventcallback_instancepass)
    {
        firstInstancePass = false;
        UIFunctions::getInstance(); // construct UIFunctions here (SIM thread)
    }

    if(message == sim_message_eventcallback_simulationended)
    { // Simulation just ended
        // TODO: move this to sim_message_eventcallback_simulationabouttoend
        // TODO: ASSERT_THREAD(???)
        Proxy::destroyTransientObjects();
    }

    static int oldSceneID = simGetInt32ParameterE(sim_intparam_scene_unique_id);
    if(message == sim_message_eventcallback_instanceswitch)
    {
        int newSceneID = simGetInt32ParameterE(sim_intparam_scene_unique_id);
        Proxy::sceneChange(oldSceneID, newSceneID);
        oldSceneID = newSceneID;
    }

    // Keep following unchanged:
    simSetIntegerParameter(sim_intparam_error_report_mode, errorModeSaved); // restore previous settings
    return retVal;
}

