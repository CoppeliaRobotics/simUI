#include "signal_spy.h"
#include "debug.h"
#include "UIProxy.h"
#include "UIFunctions.h"
#include <string>
#include <5.5.0/QtCore/private/qobject_p.h>

std::ostream& operator<< (std::ostream &out, const QByteArray &a)
{
    out << a.data();
    return out;
}

static int signalToMethodIndex(const QMetaObject * mo, int signal)
{
    Q_ASSERT(signal >= 0);
    for(int i = 0; i < mo->methodCount(); ++i)
    {
        if(mo->method(i).methodType() == QMetaMethod::Signal)
        {
            if(signal == 0) return i;
            --signal;
        }
    }
    return -1;
}

static bool spyCondition(QObject *caller)
{
    std::string className(caller->metaObject()->className());
    return className == "UIProxy" || className == "UIFunctions";
}

static std::string callerNickname(QObject *caller)
{
    if(caller == UIFunctions::getInstance()) return " (UIFunctionsSingleton)";
    if(caller == UIProxy::getInstance()) return " (UIProxySingleton)";
    return "";
}

QThreadStorage<bool> SignalSpy::entered;

void SignalSpy::signal(QObject *caller, int signalIndex, void **)
{
    if(entered.localData()) return;
    QScopedValueRollback<bool> roll(entered.localData(), true);
    if(spyCondition(caller))
    {
        int index = signalToMethodIndex(caller->metaObject(), signalIndex);
        if(index >= 0)
        {
            DEBUG_OUT << caller << callerNickname(caller) << " :: " << caller->metaObject()->method(index).methodSignature() << std::endl;
        }
    }
}

void SignalSpy::slot(QObject *caller, int index, void **)
{
    if(entered.localData()) return;
    QScopedValueRollback<bool> roll(entered.localData(), true);
    if(spyCondition(caller))
    {
        DEBUG_OUT << caller << callerNickname(caller) << " :: " << caller->metaObject()->method(index).methodSignature() << std::endl;
    }
}

void SignalSpy::start()
{
    QSignalSpyCallbackSet set = {&signal, &slot, 0L, 0L};
    qt_signal_spy_callback_set = set;
}

