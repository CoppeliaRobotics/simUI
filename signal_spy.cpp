#include "signal_spy.h"
#include <5.5.0/QtCore/private/qobject_p.h>

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
    return caller->objectName() == "UIProxy" || caller->objectName() == "UIFunctions";
}

QThreadStorage<bool> SignalSpy::entered;

void SignalSpy::signalBegin(QObject *caller, int signalIndex, void **)
{
    if(entered.localData()) return;
    QScopedValueRollback<bool> roll(entered.localData(), true);
    if(spyCondition(caller))
    {
        int index = signalToMethodIndex(caller->metaObject(), signalIndex);
        if(index >= 0)
            qDebug() << "SIGNAL" << caller << caller->metaObject()->method(index).methodSignature();
    }
}

void SignalSpy::slotBegin(QObject *caller, int index, void **)
{
    if(entered.localData()) return;
    QScopedValueRollback<bool> roll(entered.localData(), true);
    if(spyCondition(caller))
    {
        qDebug() << "SLOT" << caller << caller->metaObject()->method(index).methodSignature();
    }
}

void SignalSpy::start()
{
    QSignalSpyCallbackSet set = {&signalBegin, &slotBegin, 0L, 0L};
    qt_signal_spy_callback_set = set;
}

