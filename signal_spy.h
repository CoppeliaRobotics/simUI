#include <QtCore>

class SignalSpy {
    static QThreadStorage<bool> entered;
    static void signalBegin(QObject *caller, int signalIndex, void **);
    static void slotBegin(QObject *caller, int index, void **);
public:
   static void start();
};

