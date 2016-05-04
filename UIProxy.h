#ifndef UIPROXY_H_INCLUDED
#define UIPROXY_H_INCLUDED

#include <map>

#include <QObject>
#include <QString>
#include <QWidget>

#include "Proxy.h"
#include "widgets/Window.h"

class UIProxy : public QObject
{
    Q_OBJECT

public:
    virtual ~UIProxy();

    static UIProxy * getInstance(QObject *parent = 0);
    static void destroyInstance();

private:
    UIProxy(QObject *parent = 0);

    static UIProxy *instance;

public slots:

private slots:
    void onCreate(Proxy *proxy, Window *window);
    void onButtonClick();
    void onValueChange(int value);
    void onValueChange(QString value);
    void onDestroy(Proxy *proxy);
    void onDestroyUi(Window *window);
    void onShowWindow(Window *window);
    void onHideWindow(Window *window);

signals:
    void buttonClick(int id);
    void valueChange(int id, int value);
    void valueChange(int id, QString value);
};

#endif // UIPROXY_H_INCLUDED

