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
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QWidget>
#include <QLabel>
#include <QGroupBox>
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

#define PLUGIN_NAME "CustomUI"
#define PLUGIN_VERSION 3 // 3 since V3.3.0, 2 since V3.3.0Beta.

LIBRARY vrepLib; // the V-REP library that we will dynamically load and bind

#include "tinyxml2.h"

#include "stubs.h"
#include "LuaCallbackFunction.h"
#include "Proxy.h"
#include "UIFunctions.h"
#include "UIProxy.h"
#include "widgets/Window.h"
#include "widgets/Widget.h"

template<typename T>
std::string encodePointer(T *p, std::string prefix = "0x")
{
    if(p == NULL)
    {
        return "";
    }
    else
    {
        std::stringstream ss;
        ss << prefix << std::hex << static_cast<void *>(p);
        return ss.str();
    }
}

template<typename T>
T * decodePointer(std::string s, std::string prefix = "0x")
{
    if(boost::starts_with(s, prefix))
    {
        unsigned long x;
        std::stringstream ss;
        ss << std::hex << s.substr(prefix.size());
        ss >> x;
        return reinterpret_cast<T *>(x);
    }
    else
    {
        return NULL;
    }
}

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
    Window *window = new Window;
    std::vector<std::string> errors;
    if(!window->parse(rootElement, errors))
    {
        std::stringstream ss;
        ss << "XML tree error: ";
        for(size_t i = 0; i < errors.size(); ++i)
            ss << (i ? "; " : "") << errors[i];
#ifdef DEBUG
        std::cerr << "create: parse failed: " << ss.str() << std::endl;
#endif
        simSetLastError(cmd, ss.str().c_str());
        UIFunctions::getInstance()->destroyUi(window); // must delete on UI thread or will crash
        return;
    }

    Proxy *proxy = new Proxy(simGetSimulationState() != sim_simulation_stopped, p->scriptID);
    UIFunctions::getInstance()->create(proxy, window); // connected to UIProxy, which
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
T* getWidget(int id, const char *cmd, const char *widget_type_name)
{
    Widget *widget = Widget::byId(id);
    if(!widget)
    {
        std::stringstream ss;
        ss << "invalid widget id: " << id;
        simSetLastError(cmd, ss.str().c_str());
        return NULL;
    }

    T *qwidget = dynamic_cast<T*>(widget->getQWidget());
    if(!qwidget)
    {
        std::stringstream ss;
        ss << "invalid widget type. expected " << widget_type_name;
        simSetLastError(cmd, ss.str().c_str());
        return NULL;
    }

    return qwidget;
}

void setSliderValue(SScriptCallBack *p, const char *cmd, setSliderValue_in *in, setSliderValue_out *out)
{
    QSlider *slider = getWidget<QSlider>(in->id, cmd, "slider");
    slider->setValue(in->value);
}

void setEditValue(SScriptCallBack *p, const char *cmd, setEditValue_in *in, setEditValue_out *out)
{
    QLineEdit *edit = getWidget<QLineEdit>(in->id, cmd, "edit");
    edit->setText(QString::fromStdString(in->value));
}

void setCheckboxValue(SScriptCallBack *p, const char *cmd, setCheckboxValue_in *in, setCheckboxValue_out *out)
{
    QCheckBox *checkbox = getWidget<QCheckBox>(in->id, cmd, "checkbox");
    switch(in->value)
    {
    case 0: checkbox->setCheckState(Qt::Unchecked); break;
    case 1: checkbox->setCheckState(Qt::PartiallyChecked); break;
    case 2: checkbox->setCheckState(Qt::Checked); break;
    default: simSetLastError(cmd, "invalid checkbox value"); break;
    }
}

void setRadiobuttonValue(SScriptCallBack *p, const char *cmd, setRadiobuttonValue_in *in, setRadiobuttonValue_out *out)
{
    QRadioButton *radiobutton = getWidget<QRadioButton>(in->id, cmd, "radiobutton");
    switch(in->value)
    {
    case 0: radiobutton->setChecked(false); break;
    case 1: radiobutton->setChecked(true); break;
    default: simSetLastError(cmd, "invalid radiobutton value"); break;
    }
}

void setLabelText(SScriptCallBack *p, const char *cmd, setLabelText_in *in, setLabelText_out *out)
{
    QLabel *label = getWidget<QLabel>(in->id, cmd, "label");
    label->setText(QString::fromStdString(in->text));
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
    delete UIProxy::getInstance();
    delete UIFunctions::getInstance();

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

    if (message == sim_message_eventcallback_simulationended)
    { // Simulation just ended
        Proxy::destroyTransientObjects();
    }

    // Keep following unchanged:
    simSetIntegerParameter(sim_intparam_error_report_mode, errorModeSaved); // restore previous settings
    return(retVal);
}

