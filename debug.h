#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#include "config.h"

#include <iostream>
#include <boost/format.hpp>

#include <QThread>

#include "plugin.h"
#include "simConst.h"

extern Qt::HANDLE UI_THREAD;
extern Qt::HANDLE SIM_THREAD;

std::string threadNickname();
void uiThread();
void simThread();

void log(int v, const std::string &msg);
void log(int v, boost::format &fmt);

#ifdef __PRETTY_FUNCTION__
#define __FUNC__ __PRETTY_FUNCTION__
#else
#define __FUNC__ __func__
#endif

#define ASSERT_THREAD(ID) \
    if(UI_THREAD == NULL) {\
        log(sim_verbosity_debug, "WARNING: cannot check ASSERT_THREAD(" #ID ") because global variable UI_THREAD is not set yet.");\
    } else if(strcmp(#ID, "UI") == 0) {\
        if(QThread::currentThreadId() != UI_THREAD) {\
            log(sim_verbosity_errors, boost::format("%s:%d %s should be called from UI thread") % __FILE__ % __LINE__ % __FUNC__);\
            exit(1);\
        }\
    } else if(strcmp(#ID, "!UI") == 0) {\
        if(QThread::currentThreadId() == UI_THREAD) {\
            log(sim_verbosity_errors, boost::format("%s:%d %s should NOT be called from UI thread") % __FILE__ % __LINE__ % __FUNC__);\
            exit(1);\
        }\
    } else {\
        log(sim_verbosity_debug, "WARNING: cannot check ASSERT_THREAD(" #ID "). Can check only UI and !UI.");\
    }

#endif // DEBUG_H_INCLUDED

