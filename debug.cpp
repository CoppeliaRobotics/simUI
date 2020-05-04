#include "debug.h"
#include "simLib.h"

#include <sstream>
#include <iostream>

Qt::HANDLE UI_THREAD = NULL;
Qt::HANDLE SIM_THREAD = NULL;

std::string threadNickname()
{
    Qt::HANDLE h = QThread::currentThreadId();
    if(h == UI_THREAD) return "UI";
    if(h == SIM_THREAD) return "SIM";
    std::stringstream ss;
    ss << h;
    return ss.str();
}

void uiThread()
{
    Qt::HANDLE h = QThread::currentThreadId();
    if(UI_THREAD != NULL && UI_THREAD != h)
        std::cerr << "warning: UI thread has already been set" << std::endl;
    UI_THREAD = h;
}

void simThread()
{
    Qt::HANDLE h = QThread::currentThreadId();
    if(SIM_THREAD != NULL && SIM_THREAD != h)
        std::cerr << "warning: SIM thread has already been set" << std::endl;
    SIM_THREAD = h;
}

void log(int v, const std::string &msg)
{
    int vg = sim_verbosity_default;
    simGetModuleInfo(PLUGIN_NAME, sim_moduleinfo_verbosity, nullptr, &vg);
    if(vg < v) return;
    std::cout << PLUGIN_NAME << ": " << msg << std::endl;
}

void log(int v, boost::format &fmt)
{
    log(v, fmt.str());
}

