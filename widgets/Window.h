#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include <vector>
#include <map>
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
    std::string onclose;

    QWidget *qwidget;

    bool qwidget_geometry_saved;
    QPoint qwidget_pos;
    QSize qwidget_size;

    bool visibility_state;

    Proxy *proxy;

public:
    Window();
    virtual ~Window();

    virtual void parse(std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    std::string str();

    inline QWidget * getQWidget() {return qwidget;}

    void hide();
    void show();

    void onSceneChange(int oldSceneID, int newSceneID);

    friend class UIProxy;
    friend class UIFunctions;
    friend class QDialog2;
};

#endif // WINDOW_H_INCLUDED

