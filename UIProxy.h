#ifndef UIPROXY_H_INCLUDED
#define UIPROXY_H_INCLUDED

#include <map>

#include <QObject>
#include <QString>
#include <QLayout>

#include "tinyxml2.h"

#include "LuaCallbackFunction.h"

using tinyxml2::XMLElement;

class UIProxy : public QObject
{
    Q_OBJECT

public:
    UIProxy(QObject *parent = 0);
    virtual ~UIProxy();

    std::map<int, LuaCallbackFunction> onclickCallback;
    std::map<int, LuaCallbackFunction> onchangeCallback;

protected:
    std::map<int, QObject *> objectById;
    std::map<QObject *, int> objectId;
    int nextId = 1000000;
    QWidget * createStuff(int scriptID, QWidget *parent, XMLElement *e);

public slots:

private slots:
    void onCreate(int scriptID, QString xml);
    void onButtonClick();
    void onValueChange();

signals:
    void buttonClick(int id);
    void valueChange(int id);
};

#endif // UIPROXY_H_INCLUDED

