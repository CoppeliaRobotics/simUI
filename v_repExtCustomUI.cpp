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
#include "v_repLib.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

#include <boost/algorithm/string/predicate.hpp>

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
    DBG << "Proxy created in sceneID " << sceneID << std::endl;
    Proxy *proxy = new Proxy(destroy, sceneID, p->scriptID, window, widgets);
    UIFunctions::getInstance()->create(proxy); // connected to UIProxy, which
                            // will run code for creating Qt widgets in the UI thread
    out->uiHandle = proxy->getHandle();
}

void destroy(SScriptCallBack *p, const char *cmd, destroy_in *in, destroy_out *out)
{
    Proxy *proxy = Proxy::byHandle(in->handle);
    if(!proxy)
    {
        simSetLastError(cmd, "invalid ui handle");
        return;
    }

    UIFunctions::getInstance()->destroy(proxy); // will also delete proxy
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
    Checkbox *checkbox = getWidget<Checkbox>(in->handle, in->id, cmd, "checkbox");
    UIFunctions::getInstance()->setCheckboxValue(checkbox, in->value, in->suppressEvents);
}

void getRadiobuttonValue(SScriptCallBack *p, const char *cmd, getRadiobuttonValue_in *in, getRadiobuttonValue_out *out)
{
    QRadioButton *radiobutton = getQWidget<QRadioButton>(in->handle, in->id, cmd, "radiobutton");
    out->value = radiobutton->isChecked() ? 1 : 0;
}

void setRadiobuttonValue(SScriptCallBack *p, const char *cmd, setRadiobuttonValue_in *in, setRadiobuttonValue_out *out)
{
    Radiobutton *radiobutton = getWidget<Radiobutton>(in->handle, in->id, cmd, "radiobutton");
    UIFunctions::getInstance()->setRadiobuttonValue(radiobutton, in->value, in->suppressEvents);
}

void getLabelText(SScriptCallBack *p, const char *cmd, getLabelText_in *in, getLabelText_out *out)
{
    QLabel *label = getQWidget<QLabel>(in->handle, in->id, cmd, "label");
    out->text = label->text().toStdString();
}

void setLabelText(SScriptCallBack *p, const char *cmd, setLabelText_in *in, setLabelText_out *out)
{
    Label *label = getWidget<Label>(in->handle, in->id, cmd, "label");
    UIFunctions::getInstance()->setLabelText(label, in->text, in->suppressEvents);
}

void insertComboboxItem(SScriptCallBack *p, const char *cmd, insertComboboxItem_in *in, insertComboboxItem_out *out)
{
    Combobox *combobox = getWidget<Combobox>(in->handle, in->id, cmd, "combobox");
    UIFunctions::getInstance()->insertComboboxItem(combobox, in->index, in->text, in->suppressEvents);
}

void removeComboboxItem(SScriptCallBack *p, const char *cmd, removeComboboxItem_in *in, removeComboboxItem_out *out)
{
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

void hide(SScriptCallBack *p, const char *cmd, hide_in *in, hide_out *out)
{
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
    Proxy *proxy = Proxy::byHandle(in->handle);
    if(!proxy)
    {
        simSetLastError(cmd, "invalid ui handle");
        return;
    }

    Window *window = proxy->getWidget();
    UIFunctions::getInstance()->setPosition(window, in->x, in->y);
}

void setImageData(SScriptCallBack *p, const char *cmd, setImageData_in *in, setImageData_out *out)
{
    Image *imageWidget = dynamic_cast<Image*>(Widget::byId(in->handle, in->id));
    if(!imageWidget)
    {
        std::stringstream ss;
        ss << "invalid image widget id: " << in->id;
        throw std::runtime_error(ss.str());
    }

    QImage::Format format = QImage::Format_RGB888;
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
    Widget *widget = Widget::byId(in->handle, in->id);
    if(!widget)
    {
        std::stringstream ss;
        ss << "invalid widget id: " << in->id;
        throw std::runtime_error(ss.str());
    }

    UIFunctions::getInstance()->setEnabled(widget, in->enabled);
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
    if (vrepLib == NULL)
    {
        std::cout << "Error, could not find or correctly load the V-REP library. Cannot start '" PLUGIN_NAME "' plugin.\n";
        return(0);
    }
    if (getVrepProcAddresses(vrepLib)==0)
    {
        std::cout << "Error, could not find all required functions in the V-REP library. Cannot start '" PLUGIN_NAME "' plugin.\n";
        unloadVrepLibrary(vrepLib);
        return(0);
    }

    int vrepVer;
    simGetIntegerParameter(sim_intparam_program_version, &vrepVer);
    if (vrepVer < 30203) // if V-REP version is smaller than 3.02.03
    {
        std::cout << "Sorry, your V-REP copy is somewhat old. Cannot start '" PLUGIN_NAME "' plugin.\n";
        unloadVrepLibrary(vrepLib);
        return(0);
    }

    if(!registerScriptStuff())
    {
        std::cout << "Initialization failed.\n";
        unloadVrepLibrary(vrepLib);
        return(0);
    }

    UIProxy::getInstance(); // construct UIProxy here (UI thread)

    return(PLUGIN_VERSION); // initialization went fine, we return the version number of this plugin (can be queried with simGetModuleName)
}

VREP_DLLEXPORT void v_repEnd()
{
    Proxy::destroyAllObjects();

    UIFunctions::destroyInstance();
    UIProxy::destroyInstance();

    unloadVrepLibrary(vrepLib); // release the library
}

VREP_DLLEXPORT void* v_repMessage(int message, int* auxiliaryData, void* customData, int* replyData)
{
    // Keep following 5 lines at the beginning and unchanged:
    static bool refreshDlgFlag = true;
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
        Proxy::destroyTransientObjects();
    }

#ifdef VREP_INSTANCE_SWITCH_WORKS
    static int oldSceneID = simGetInt32ParameterE(sim_intparam_scene_unique_id);
    if(message == sim_message_eventcallback_instanceswitch)
    {
        int newSceneID = simGetInt32ParameterE(sim_intparam_scene_unique_id);
        Proxy::sceneChange(oldSceneID, newSceneID);
        oldSceneID = newSceneID;
    }
#else
    // XXX: currently (3.3.1 beta) it is broken
    if(message == sim_message_eventcallback_instancepass)
    {
        static int oldSceneID = -1;
        if(oldSceneID == -1) oldSceneID = simGetInt32ParameterE(sim_intparam_scene_unique_id);
        int sceneID = simGetInt32ParameterE(sim_intparam_scene_unique_id);
        if(sceneID != oldSceneID)
        {
            Proxy::sceneChange(oldSceneID, sceneID);
            oldSceneID = sceneID;
        }
    }
#endif

    // Keep following unchanged:
    simSetIntegerParameter(sim_intparam_error_report_mode, errorModeSaved); // restore previous settings
    return(retVal);
}

