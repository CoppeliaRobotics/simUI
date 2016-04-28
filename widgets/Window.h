#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include <vector>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UIProxy;

#include "LayoutWidget.h"

class Window : public LayoutWidget
{
protected:
    std::string title;
    bool resizable;
    bool closeable;

    QWidget *qwidget;

    Proxy *proxy;

public:
    Window();
    virtual ~Window();

    virtual void parse(tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    std::string str();

    inline QWidget * getQWidget() {return qwidget;}

    friend class UIProxy;
};

#endif // WINDOW_H_INCLUDED

