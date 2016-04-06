#ifndef MYOBJECT_H_INCLUDED
#define MYOBJECT_H_INCLUDED

#include <QObject>
#include <QString>

#include "UIProxy.h"

class UIFunctions : public QObject
{
    Q_OBJECT

public:
    UIFunctions(QObject *parent = 0);
    virtual ~UIFunctions();

    void connectToProxy(UIProxy *uiproxy);

signals:
    void createWindow(QString xml);
};

#endif // MYOBJECT_H_INCLUDED

