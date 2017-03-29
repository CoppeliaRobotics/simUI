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
#include "signal_spy.h"
#include "v_repLib.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

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
#include "LuaCallbackFunction.h"
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

template<typename T>
T* getWidget(int handle, int id, const char *cmd, const char *widget_type_name)
{
    Widget *widget = Widget::byId(handle, id);
    if(!widget)
    {
        std::stringstream ss;
        ss << "invalid widget id: " << id;
        throw std::runtime_error(ss.str());
    }

    T *twidget = dynamic_cast<T*>(widget);
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

void getSliderValue(SScriptCallBack *p, const char *cmd, getSliderValue_in *in, getSliderValue_out *out)
{
    QSlider *slider = getQWidget<QSlider>(in->handle, in->id, cmd, "slider");
    out->value = slider->value();
}

void setSliderValue(SScriptCallBack *p, const char *cmd, setSliderValue_in *in, setSliderValue_out *out)
{
    ASSERT_THREAD(!UI);
    Slider *slider = getWidget<Slider>(in->handle, in->id, cmd, "slider");
    UIFunctions::getInstance()->setSliderValue(slider, in->value, in->suppressEvents);
}

void getEditValue(SScriptCallBack *p, const char *cmd, getEditValue_in *in, getEditValue_out *out)
{
    QLineEdit *edit = getQWidget<QLineEdit>(in->handle, in->id, cmd, "edit");
    out->value = edit->text().toStdString();
}

void setEditValue(SScriptCallBack *p, const char *cmd, setEditValue_in *in, setEditValue_out *out)
{
    ASSERT_THREAD(!UI);
    Edit *edit = getWidget<Edit>(in->handle, in->id, cmd, "edit");
    UIFunctions::getInstance()->setEditValue(edit, in->value, in->suppressEvents);
}

void getSpinboxValue(SScriptCallBack *p, const char *cmd, getSpinboxValue_in *in, getSpinboxValue_out *out)
{
    QAbstractSpinBox *aspinbox = getQWidget<QAbstractSpinBox>(in->handle, in->id, cmd, "spinbox");
    QSpinBox *spinbox = dynamic_cast<QSpinBox*>(aspinbox);
    QDoubleSpinBox *doubleSpinbox = dynamic_cast<QDoubleSpinBox*>(aspinbox);
    out->value = spinbox ? spinbox->value() : doubleSpinbox ? doubleSpinbox->value() : 0;
}

void setSpinboxValue(SScriptCallBack *p, const char *cmd, setSpinboxValue_in *in, setSpinboxValue_out *out)
{
    ASSERT_THREAD(!UI);
    Spinbox *spinbox = getWidget<Spinbox>(in->handle, in->id, cmd, "spinbox");
    UIFunctions::getInstance()->setSpinboxValue(spinbox, in->value, in->suppressEvents);
}

void getCheckboxValue(SScriptCallBack *p, const char *cmd, getCheckboxValue_in *in, getCheckboxValue_out *out)
{
    QCheckBox *checkbox = getQWidget<QCheckBox>(in->handle, in->id, cmd, "checkbox");
    switch(checkbox->checkState())
    {
    case Qt::Unchecked: out->value = 0; break;
    case Qt::PartiallyChecked: out->value = 1; break;
    case Qt::Checked: out->value = 2; break;
    default: simSetLastError(cmd, "invalid checkbox value"); break;
    }
}

void setCheckboxValue(SScriptCallBack *p, const char *cmd, setCheckboxValue_in *in, setCheckboxValue_out *out)
{
    ASSERT_THREAD(!UI);
    Checkbox *checkbox = getWidget<Checkbox>(in->handle, in->id, cmd, "checkbox");
    switch(in->value)
    {
    case 0:
    case 1:
    case 2:
        break;
    default:
        simSetLastError(cmd, "invalid checkbox value. must me 0, 1 or 2");
        return;
    }
    UIFunctions::getInstance()->setCheckboxValue(checkbox, in->value, in->suppressEvents);
}

void getRadiobuttonValue(SScriptCallBack *p, const char *cmd, getRadiobuttonValue_in *in, getRadiobuttonValue_out *out)
{
    QRadioButton *radiobutton = getQWidget<QRadioButton>(in->handle, in->id, cmd, "radiobutton");
    out->value = radiobutton->isChecked() ? 1 : 0;
}

void setRadiobuttonValue(SScriptCallBack *p, const char *cmd, setRadiobuttonValue_in *in, setRadiobuttonValue_out *out)
{
    ASSERT_THREAD(!UI);
    Radiobutton *radiobutton = getWidget<Radiobutton>(in->handle, in->id, cmd, "radiobutton");
    switch(in->value)
    {
    case 0:
    case 1:
        break;
    default:
        simSetLastError(cmd, "invalid radiobutton value. must me 0 or 1");
        return;
    }
    UIFunctions::getInstance()->setRadiobuttonValue(radiobutton, in->value, in->suppressEvents);
}

void getLabelText(SScriptCallBack *p, const char *cmd, getLabelText_in *in, getLabelText_out *out)
{
    QLabel *label = getQWidget<QLabel>(in->handle, in->id, cmd, "label");
    out->text = label->text().toStdString();
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
    QComboBox *combobox = getQWidget<QComboBox>(in->handle, in->id, cmd, "combobox");
    out->count = combobox->count();
}

void getComboboxItemText(SScriptCallBack *p, const char *cmd, getComboboxItemText_in *in, getComboboxItemText_out *out)
{
    QComboBox *combobox = getQWidget<QComboBox>(in->handle, in->id, cmd, "combobox");
    out->text = combobox->itemText(in->index).toStdString();
}

void getComboboxItems(SScriptCallBack *p, const char *cmd, getComboboxItems_in *in, getComboboxItems_out *out)
{
    QComboBox *combobox = getQWidget<QComboBox>(in->handle, in->id, cmd, "combobox");
    for(int i = 0; i < combobox->count(); ++i)
        out->items.push_back(combobox->itemText(i).toStdString());
}

void setComboboxItems(SScriptCallBack *p, const char *cmd, setComboboxItems_in *in, setComboboxItems_out *out)
{
    ASSERT_THREAD(!UI);
    Combobox *combobox = getWidget<Combobox>(in->handle, in->id, cmd, "combobox");
    QStringList items;
    BOOST_FOREACH(std::string &s, in->items)
        items.push_back(QString::fromStdString(s));
    UIFunctions::getInstance()->setComboboxItems(combobox, items, in->index, in->suppressEvents);
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

VREP_DLLEXPORT unsigned char v_repStart(void* reservedPointer, int reservedInt)
{
    char curDirAndFile[1024];
#ifdef _WIN32
    #ifdef QT_COMPIL
        _getcwd(curDirAndFile, sizeof(curDirAndFile));
    #else
        GetModuleFileName(NULL, curDirAndFile, 1023);
        PathRemoveFileSpec(curDirAndFile);
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

#ifdef DEBUG
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

