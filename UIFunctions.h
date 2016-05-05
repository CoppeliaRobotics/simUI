#ifndef MYOBJECT_H_INCLUDED
#define MYOBJECT_H_INCLUDED

#include <QObject>
#include <QString>

#include "Proxy.h"
#include "widgets/Window.h"
#include "widgets/Image.h"

class UIFunctions : public QObject
{
    Q_OBJECT

public:
    virtual ~UIFunctions();

    static UIFunctions * getInstance(QObject *parent = 0);
    static void destroyInstance();

private:
    UIFunctions(QObject *parent = 0);

    static UIFunctions *instance;

public slots:

private slots:
    void onButtonClick(int id);
    void onValueChange(int id, int value);
    void onValueChange(int id, QString value);

signals:
    void create(Proxy *proxy);
    void destroy(Proxy *proxy);
    void destroyUi(Window *window);
    void showWindow(Window *window);
    void hideWindow(Window *window);
    void setImage(Image *image, const char *data, int w, int h);
};

#endif // MYOBJECT_H_INCLUDED

