#ifndef MYOBJECT_H_INCLUDED
#define MYOBJECT_H_INCLUDED

#include <QObject>
#include <QString>

#include "Proxy.h"
#include "UIProxy.h"
#include "UIModel.h"

class UIFunctions : public QObject
{
    Q_OBJECT

public:
    virtual ~UIFunctions();

    static UIFunctions * getInstance(QObject *parent = 0);

private:
    UIFunctions(QObject *parent = 0);

    static UIFunctions *instance;

public slots:

private slots:
    void onButtonClick(int id);
    void onValueChange(int id, int value);
    void onValueChange(int id, QString value);

signals:
    void create(Proxy *proxy, Window *window);
    void destroy(Proxy *proxy);
};

#endif // MYOBJECT_H_INCLUDED

