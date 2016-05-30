#include "debug.h"

#include <sstream>

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
        throw std::string("UI thread has already been set");
    UI_THREAD = h;
}

void simThread()
{
    Qt::HANDLE h = QThread::currentThreadId();
    if(SIM_THREAD != NULL && SIM_THREAD != h)
        throw std::string("SIM thread has already been set");
    SIM_THREAD = h;
}

