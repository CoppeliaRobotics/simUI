#ifndef UIPROXY_H_INCLUDED
#define UIPROXY_H_INCLUDED

#include <map>

#include <QObject>
#include <QString>
#include <QWidget>

#include "Proxy.h"
#include "widgets/Window.h"
#include "widgets/Image.h"

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
    void onCreate(Proxy *proxy);
    void onButtonClick();
    void onValueChange(int value);
    void onValueChange(QString value);
    void onDestroy(Proxy *proxy);
    void onDestroyUi(Window *window);
    void onShowWindow(Window *window);
    void onHideWindow(Window *window);
    void onSetImage(Image *image, const char *data, int w, int h);
    void onSceneChange(Window *window, int oldSceneID, int newSceneID);

signals:
    void buttonClick(Widget *widget);
    void valueChange(Widget *widget, int value);
    void valueChange(Widget *widget, QString value);
    void windowClose(Window *window);
    void loadImageFromFile(Image *image, const char *filename, int w, int h);
};

#endif // UIPROXY_H_INCLUDED

