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

    QWidget *qwidget;

    Proxy *proxy;

public:
    Window();
    virtual ~Window();

    virtual bool parse(tinyxml2::XMLElement *e, std::vector<std::string>& errors);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class UIProxy;
};

#endif // WINDOW_H_INCLUDED

