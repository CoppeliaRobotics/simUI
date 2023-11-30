#include "signal_spy.h"
#include "UI.h"
#include "SIM.h"
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
    return className == "UI" || className == "SIM";
}

static std::string callerNickname(QObject *caller)
{
    if(caller == SIM::getInstance()) return " (SIM)";
    if(caller == UI::getInstance()) return " (UI)";
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
            log(sim_verbosity_debug, boost::format("%x %s :: %s") % caller % callerNickname(caller) % caller->metaObject()->method(index).methodSignature());
        }
    }
}

void SignalSpy::slot(QObject *caller, int index, void **)
{
    if(entered.localData()) return;
    QScopedValueRollback<bool> roll(entered.localData(), true);
    if(spyCondition(caller))
    {
        log(sim_verbosity_debug, boost::format("%x %s :: %s") % caller % callerNickname(caller) % caller->metaObject()->method(index).methodSignature());
    }
}

void SignalSpy::start()
{
    QSignalSpyCallbackSet set = {&signal, &slot, 0L, 0L};
    qt_signal_spy_callback_set = set;
}

