#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#include <iostream>

#include <QThread>

#include "plugin.h"

#ifdef QT_DEBUG
#define DEBUG
#endif

extern Qt::HANDLE UI_THREAD;
extern Qt::HANDLE SIM_THREAD;

std::string threadNickname();
void uiThread();
void simThread();

#ifndef DEBUG_STREAM
#define DEBUG_STREAM std::cerr
#endif // DEBUG_STREAM

#ifdef __PRETTY_FUNCTION__
#define DBG_WHAT __PRETTY_FUNCTION__
#else
#define DBG_WHAT __func__
#endif

#ifdef DEBUG
#define DBG DEBUG_STREAM << "\033[1;33m[" << PLUGIN_NAME << ":" << threadNickname() << "] \033[1;31m" << __FILE__ << ":" << __LINE__ << "  \033[1;32m" << DBG_WHAT << "\033[0m" << "  "
#else // DEBUG
#define DBG if(true) {} else DEBUG_STREAM
#endif // DEBUG

#define ASSERT_THREAD(ID) if(ID##_THREAD != NULL && QThread::currentThreadId() != ID##_THREAD) { std::cerr << PLUGIN_NAME << ": " << __FILE__ << ":" << __LINE__ << " FATAL: " << DBG_WHAT << " should be called from " #ID " thread" << std::endl; exit(1); } else if(ID##_THREAD == NULL) { DBG << "WARNING: cannot check ASSERT_THREAD(" #ID ") because global variable " #ID "_THREAD is not set." << std::endl; }

#endif // DEBUG_H_INCLUDED

