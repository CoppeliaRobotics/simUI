#ifndef UIPROXY_H_INCLUDED
#define UIPROXY_H_INCLUDED

#include <map>

#include <QObject>
#include <QString>
#include <QWidget>

#include "Proxy.h"
#include "UIModel.h"

class UIProxy : public QObject
{
    Q_OBJECT

public:
    UIProxy(QObject *parent = 0);
    virtual ~UIProxy();

public slots:

private slots:
    void onCreate(Proxy *proxy, Window *window);
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

