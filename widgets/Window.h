#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include "config.h"

#include <vector>
#include <map>
#include <set>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UI;

#include "LayoutWidget.h"

class WindowWidget : public Widget
{
    WindowWidget();
    QWidget * createQtWidget(Proxy *proxy, UI *ui, QWidget *parent);

    friend class Window;
};

class Window : public LayoutWidget
{
protected:
    bool enabled;
    std::string title;
    bool resizable;
    bool closeable;
    bool modal;
    std::string onclose;
    std::string style;
    bool activate;
    std::string placement;

    QWidget *qwidget;

    bool qwidget_geometry_saved;
    QPoint qwidget_pos;
    QSize qwidget_size;

    bool visibility_state;

    Proxy *proxy;

    static std::set<Window *> windows;

public:
    Window();
    virtual ~Window();

    virtual void parse(std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UI *ui, QWidget *parent);

    std::string str();

    inline QWidget * getQWidget() {return qwidget;}

    void hide();
    void show();
    void move(int x, int y);
    void resize(int w, int h);
    void setTitle(std::string title);
    std::string getTitle();
    void setEnabled(bool enabled);

    void onSceneChange(int oldSceneID, int newSceneID);

    static bool exists(Window *w);

    friend class UI;
    friend class SIM;
    friend class QDialog2;
};

#endif // WINDOW_H_INCLUDED

