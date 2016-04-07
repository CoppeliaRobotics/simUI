#ifndef UIPROXY_H_INCLUDED
#define UIPROXY_H_INCLUDED

#include <map>

#include <QObject>
#include <QString>
#include <QWidget>

#include "tinyxml2.h"

#include "LuaCallbackFunction.h"
#include "Proxy.h"

using tinyxml2::XMLElement;

class UIProxy : public QObject
{
    Q_OBJECT

public:
    UIProxy(QObject *parent = 0);
    virtual ~UIProxy();

protected:
    std::map<int, QObject *> objectById;
    std::map<QObject *, int> objectId;
    int nextId;
    QWidget * createStuff(Proxy *proxy, int scriptID, QWidget *parent, XMLElement *e);

public slots:

private slots:
    void onCreate(Proxy *proxy, int scriptID, QString xml);
    void onButtonClick();
    void onValueChange(int value);
    void onValueChange(QString value);
    void onDestroy(Proxy *proxy);

signals:
    void buttonClick(int id);
    void valueChange(int id, int value);
    void valueChange(int id, QString value);
};

#endif // UIPROXY_H_INCLUDED

