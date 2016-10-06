#include <QtCore>

class SignalSpy {
    static QThreadStorage<bool> entered;
    static void signal(QObject *caller, int signalIndex, void **);
    static void slot(QObject *caller, int index, void **);
public:
   static void start();
};

