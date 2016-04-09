#ifndef UI_MODEL_H_INCLUDED
#define UI_MODEL_H_INCLUDED

#include <vector>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UIProxy;

using tinyxml2::XMLElement;

enum Layout
{
    VBOX = 1,
    HBOX,
    GRID,
    FORM
};

struct Widget
{
protected:
    static int nextId;
    static std::map<int, Widget *> widgets;
    static std::map<QWidget *, Widget *> widgetByQWidget;

    int id;

    QWidget *qwidget;

    Proxy *proxy;

    Widget();

public:
    virtual ~Widget();

    virtual bool parse(XMLElement *e, std::vector<std::string>& errors);
    virtual QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent) = 0;

    inline QWidget * getQWidget() {return qwidget;}

    template<typename T>
    static Widget * tryParse(XMLElement *e, std::vector<std::string>& errors);
    static Widget * parseAny(XMLElement *e, std::vector<std::string>& errors);

    static Widget * byId(int id);
    static Widget * byQWidget(QWidget *w);

    friend class UIFunctions;
    friend class UIProxy;
    friend struct Window;
};

struct LayoutWidget
{
protected:
    Layout layout;
    std::vector< std::vector<Widget*> > children;

public:
    bool parse(XMLElement *e, std::vector<std::string>& errors);    
    void createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);
};

struct Button : public Widget
{
protected:
    std::string text;
    std::string onclick;

public:
    Button();
    virtual ~Button();

    bool parse(XMLElement *e, std::vector<std::string>& errors);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class UIFunctions;
};

struct Edit : public Widget
{
protected:
    std::string onchange;

public:
    Edit();
    virtual ~Edit();

    bool parse(XMLElement *e, std::vector<std::string>& errors);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class UIFunctions;
};

struct HSlider : public Widget
{
protected:
    std::string onchange;
    int minimum;
    int maximum;

public:
    HSlider();
    virtual ~HSlider();

    bool parse(XMLElement *e, std::vector<std::string>& errors);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class UIFunctions;
};

struct VSlider : public Widget
{
protected:
    std::string onchange;
    int minimum;
    int maximum;

public:
    VSlider();
    virtual ~VSlider();

    bool parse(XMLElement *e, std::vector<std::string>& errors);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class UIFunctions;
};

struct Label : public Widget
{
protected:
    std::string text;

public:
    Label();
    virtual ~Label();

    bool parse(XMLElement *e, std::vector<std::string>& errors);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class UIFunctions;
};

struct Checkbox : public Widget
{
protected:
    std::string text;
    std::string onchange;

public:
    Checkbox();
    virtual ~Checkbox();

    bool parse(XMLElement *e, std::vector<std::string>& errors);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class UIFunctions;
};

struct Radiobutton : public Widget
{
protected:
    std::string text;
    std::string onclick;

public:
    Radiobutton();
    virtual ~Radiobutton();

    bool parse(XMLElement *e, std::vector<std::string>& errors);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class UIFunctions;
};

struct Spinbox : public Widget
{
protected:
    std::string onchange;
    int minimum;
    int maximum;
    std::string prefix;
    std::string suffix;
    int step;

public:
    Spinbox();
    virtual ~Spinbox();

    bool parse(XMLElement *e, std::vector<std::string>& errors);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class UIFunctions;
};

struct Group : public Widget, public LayoutWidget
{
protected:
    std::string text;

public:
    Group();
    virtual ~Group();

    bool parse(XMLElement *e, std::vector<std::string>& errors);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class UIFunctions;
};

struct Window : public LayoutWidget
{
protected:
    std::string title;

    QWidget *qwidget;

    Proxy *proxy;

public:
    Window();
    virtual ~Window();

    virtual bool parse(XMLElement *e, std::vector<std::string>& errors);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class UIProxy;
};

#endif // UI_MODEL_H_INCLUDED

