#include "UIModel.h"
#include "UIProxy.h"

#include <cstring>
#include <iostream>
#include <sstream>

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QSlider>
#include <QLabel>
#include <QCheckBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QGroupBox>
#include <QDialog>
#include <QLayout>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFormLayout>

using tinyxml2::XML_NO_ERROR;

int Widget::nextId = 1000000;
std::map<int, Widget *> Widget::widgets;
std::map<QWidget *, Widget *> Widget::widgetByQWidget;

bool LayoutWidget::parse(XMLElement *e, std::vector<std::string>& errors)
{
    if(e->Attribute("layout"))
    {
        std::string layoutName(e->Attribute("layout"));
        if(layoutName == "vbox") layout = VBOX;
        else if(layoutName == "hbox") layout = HBOX;
        else if(layoutName == "grid") layout = GRID;
        else if(layoutName == "form") layout = FORM;
        else
        {
            errors.push_back("invalid value '" + layoutName + "' for attribute 'layout'");
            return false;
        }
    }
    else layout = VBOX;

    std::vector<Widget*> row;
    for(XMLElement *e1 = e->FirstChildElement(); e1; e1 = e1->NextSiblingElement())
    {
        std::string tag1(e1->Value());

        if(tag1 == "br" && layout == GRID)
        {
            children.push_back(row);
            row.clear();
            continue;
        }

        Widget *w = Widget::parseAny(e1, errors);
        if(!w)
        {
            children.push_back(row); // push widget created until now so they won't leak
            return false;
        }
        row.push_back(w);

        if((layout == FORM && row.size() == 2) ||
                layout == VBOX ||
                layout == HBOX)
        {
            children.push_back(row);
            row.clear();
        }
    }
    if(row.size() > 0 && layout == GRID)
    {
        children.push_back(row);
        row.clear();
    }
    if(row.size() > 0)
    {
        errors.push_back("extra elements in layout");
        children.push_back(row); // push widget created until now so they won't leak
        return false;
    }

    return true;
}

void LayoutWidget::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    switch(layout)
    {
    case VBOX:
    case HBOX:
        {
            QBoxLayout *qlayout;
            if(layout == VBOX) qlayout = new QVBoxLayout(parent);
            if(layout == HBOX) qlayout = new QHBoxLayout(parent);
            for(std::vector< std::vector<Widget*> >::iterator it = children.begin(); it != children.end(); ++it)
            {
                QWidget *w = (*it)[0]->createQtWidget(proxy, uiproxy, parent);
                qlayout->addWidget(w);
            }
            parent->setLayout(qlayout);
        }
        break;
    case GRID:
        {
            QGridLayout *qlayout = new QGridLayout(parent);
            int row = 0, col = 0;
            for(std::vector< std::vector<Widget*> >::iterator it = children.begin(); it != children.end(); ++it)
            {
                col = 0;
                for(std::vector<Widget*>::iterator it2 = it->begin(); it2 != it->end(); ++it2)
                {
                    QWidget *w = (*it2)->createQtWidget(proxy, uiproxy, parent);
                    qlayout->addWidget(w, row, col);
                    col++;
                }
                row++;
            }
            parent->setLayout(qlayout);
        }
        break;
    case FORM:
        {
            QFormLayout *qlayout = new QFormLayout(parent);
            for(std::vector< std::vector<Widget*> >::iterator it = children.begin(); it != children.end(); ++it)
            {
                QWidget *w1 = (*it)[0]->createQtWidget(proxy, uiproxy, parent);
                QWidget *w2 = (*it)[1]->createQtWidget(proxy, uiproxy, parent);
                qlayout->addRow(w1, w2);
            }
            parent->setLayout(qlayout);
        }
        break;
    }
}

Widget::Widget()
    : qwidget(NULL)
{
    // don't do this here because id is set by user:
    // Widget::widgets[id] = this;
}

Widget::~Widget()
{
#ifdef DEBUG
    std::cerr << "Widget::~Widget() - this=" << std::hex << ((void*)this) << std::dec << std::endl;
#endif

    // this should be destroyed from the UI thread

    if(qwidget)
    {
#ifdef DEBUG
    std::cerr << "Widget::~Widget() - delete 'qwidget' member (deleteLater())" << std::endl;
#endif

        qwidget->deleteLater();

        Widget::widgetByQWidget.erase(qwidget);
    }

    Widget::widgets.erase(id);
}

Widget * Widget::byId(int id)
{
    std::map<int, Widget*>::const_iterator it = Widget::widgets.find(id);
    Widget *ret = it == Widget::widgets.end() ? NULL : it->second;

#ifdef DEBUG
    std::cerr << "Widget::byId(" << id << ") -> " << std::hex << ret << std::dec << std::endl;
#endif

    return ret;
}

Widget * Widget::byQWidget(QWidget *w)
{
    std::map<QWidget*, Widget*>::const_iterator it = Widget::widgetByQWidget.find(w);
    Widget *ret = it == Widget::widgetByQWidget.end() ? NULL : it->second;

#ifdef DEBUG
    std::cerr << "Widget::byQWidget(" << std::hex << w << std::dec << ") -> " << std::hex << ret << std::dec << std::endl;
#endif

    return ret;
}

template<typename T>
Widget * Widget::tryParse(XMLElement *e, std::vector<std::string>& errors)
{
    T *obj = new T;
    if(obj->parse(e, errors))
    {
        Widget::widgets[obj->id] = obj;
        return obj;
    }
    else
    {
        delete obj;
        return NULL;
    }
}

Widget * Widget::parseAny(XMLElement *e, std::vector<std::string>& errors)
{
    std::string tag(e->Value());
    Widget *w = NULL;
    if(tag == "button" && (w = tryParse<Button>(e, errors))) return w;
    if(tag == "edit" && (w = tryParse<Edit>(e, errors))) return w;
    if(tag == "hslider" && (w = tryParse<HSlider>(e, errors))) return w;
    if(tag == "vslider" && (w = tryParse<VSlider>(e, errors))) return w;
    if(tag == "label" && (w = tryParse<Label>(e, errors))) return w;
    if(tag == "checkbox" && (w = tryParse<Checkbox>(e, errors))) return w;
    if(tag == "radiobutton" && (w = tryParse<Radiobutton>(e, errors))) return w;
    if(tag == "spinbox" && (w = tryParse<Spinbox>(e, errors))) return w;
    if(tag == "group" && (w = tryParse<Group>(e, errors))) return w;

    std::stringstream ss;
    ss << "could not parse <" << tag << ">";
    errors.push_back(ss.str());
    return w;
}

bool Widget::parse(XMLElement *e, std::vector<std::string>& errors)
{
    if(!e->Attribute("id") || e->QueryIntAttribute("id", &id) != XML_NO_ERROR)
    {
        id = nextId++;
    }

    return true;
}

Button::Button()
    : Widget()
{
}

Button::~Button()
{
}

bool Button::parse(XMLElement *e, std::vector<std::string>& errors)
{
    if(!Widget::parse(e, errors)) return false;

    std::string tag(e->Value());
    if(tag != "button")
    {
        errors.push_back("element must be <button>");
        return false;
    }

    if(e->Attribute("text")) text = e->Attribute("text");
    else text = "???";

    if(e->Attribute("onclick")) onclick = e->Attribute("onclick");
    else onclick = "";

    return true;
}

QWidget * Button::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QPushButton *button = new QPushButton(QString::fromStdString(text), parent);
    QObject::connect(button, SIGNAL(released()), uiproxy, SLOT(onButtonClick()));
    qwidget = button;
    Widget::widgetByQWidget[qwidget] = this;
    this->proxy = proxy;
    return button;
}

Edit::Edit()
    : Widget()
{
}

Edit::~Edit()
{
}

bool Edit::parse(XMLElement *e, std::vector<std::string>& errors)
{
    if(!Widget::parse(e, errors)) return false;

    std::string tag(e->Value());
    if(tag != "edit")
    {
        errors.push_back("element must be <edit>");
        return false;
    }

    if(e->Attribute("onchange")) onchange = e->Attribute("onchange");
    else onchange = "";

    return true;
}

QWidget * Edit::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QLineEdit *edit = new QLineEdit(parent);
    QObject::connect(edit, SIGNAL(textChanged(QString)), uiproxy, SLOT(onValueChange(QString)));
    qwidget = edit;
    Widget::widgetByQWidget[qwidget] = this;
    this->proxy = proxy;
    return edit;
}

HSlider::HSlider()
    : Widget()
{
}

HSlider::~HSlider()
{
}

bool HSlider::parse(XMLElement *e, std::vector<std::string>& errors)
{
    if(!Widget::parse(e, errors)) return false;

    std::string tag(e->Value());
    if(tag != "hslider")
    {
        errors.push_back("element must be <hslider>");
        return false;
    }

    if(!e->Attribute("minimum") || e->QueryIntAttribute("minimum", &minimum) != XML_NO_ERROR)
        minimum = 0;

    if(!e->Attribute("maximum") || e->QueryIntAttribute("maximum", &maximum) != XML_NO_ERROR)
        maximum = 100;

    if(e->Attribute("onchange")) onchange = e->Attribute("onchange");
    else onchange = "";

    return true;
}

QWidget * HSlider::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QSlider *slider = new QSlider(Qt::Horizontal, parent);
    slider->setMinimum(minimum);
    slider->setMaximum(maximum);
    QObject::connect(slider, SIGNAL(valueChanged(int)), uiproxy, SLOT(onValueChange(int)));
    qwidget = slider;
    Widget::widgetByQWidget[qwidget] = this;
    this->proxy = proxy;
    return slider;
}

VSlider::VSlider()
    : Widget()
{
}

VSlider::~VSlider()
{
}

bool VSlider::parse(XMLElement *e, std::vector<std::string>& errors)
{
    if(!Widget::parse(e, errors)) return false;

    std::string tag(e->Value());
    if(tag != "vslider")
    {
        errors.push_back("element must be <vslider>");
        return false;
    }

    if(!e->Attribute("minimum") || e->QueryIntAttribute("minimum", &minimum) != XML_NO_ERROR)
        minimum = 0;

    if(!e->Attribute("maximum") || e->QueryIntAttribute("maximum", &maximum) != XML_NO_ERROR)
        maximum = 100;

    if(e->Attribute("onchange")) onchange = e->Attribute("onchange");
    else onchange = "";

    return true;
}

QWidget * VSlider::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QSlider *slider = new QSlider(Qt::Vertical, parent);
    slider->setMinimum(minimum);
    slider->setMaximum(maximum);
    QObject::connect(slider, SIGNAL(valueChanged(int)), uiproxy, SLOT(onValueChange(int)));
    qwidget = slider;
    Widget::widgetByQWidget[qwidget] = this;
    this->proxy = proxy;
    return slider;
}

Label::Label()
    : Widget()
{
}

Label::~Label()
{
}

bool Label::parse(XMLElement *e, std::vector<std::string>& errors)
{
    if(!Widget::parse(e, errors)) return false;

    std::string tag(e->Value());
    if(tag != "label")
    {
        errors.push_back("element must be <label>");
        return false;
    }

    if(e->Attribute("text")) text = e->Attribute("text");
    else text = "";

    return true;
}

QWidget * Label::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QLabel *label = new QLabel(QString::fromStdString(text), parent);
    qwidget = label;
    Widget::widgetByQWidget[qwidget] = this;
    this->proxy = proxy;
    return label;
}

Checkbox::Checkbox()
    : Widget()
{
}

Checkbox::~Checkbox()
{
}

bool Checkbox::parse(XMLElement *e, std::vector<std::string>& errors)
{
    if(!Widget::parse(e, errors)) return false;

    std::string tag(e->Value());
    if(tag != "checkbox")
    {
        errors.push_back("element must be <checkbox>");
        return false;
    }

    if(e->Attribute("text")) text = e->Attribute("text");
    else text = "???";

    if(e->Attribute("onchange")) onchange = e->Attribute("onchange");
    else onchange = "";

    return true;
}

QWidget * Checkbox::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QCheckBox *checkbox = new QCheckBox(QString::fromStdString(text), parent);
    QObject::connect(checkbox, SIGNAL(stateChanged(int)), uiproxy, SLOT(onValueChange(int)));
    qwidget = checkbox;
    Widget::widgetByQWidget[qwidget] = this;
    this->proxy = proxy;
    return checkbox;
}

Radiobutton::Radiobutton()
    : Widget()
{
}

Radiobutton::~Radiobutton()
{
}

bool Radiobutton::parse(XMLElement *e, std::vector<std::string>& errors)
{
    if(!Widget::parse(e, errors)) return false;

    std::string tag(e->Value());
    if(tag != "radiobutton")
    {
        errors.push_back("element must be <radiobutton>");
        return false;
    }

    if(e->Attribute("text")) text = e->Attribute("text");
    else text = "";

    if(e->Attribute("onclick")) onclick = e->Attribute("onclick");
    else onclick = "";

    return true;
}

QWidget * Radiobutton::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QRadioButton *button = new QRadioButton(QString::fromStdString(text), parent);
    QObject::connect(button, SIGNAL(released()), uiproxy, SLOT(onButtonClick()));
    qwidget = button;
    Widget::widgetByQWidget[qwidget] = this;
    this->proxy = proxy;
    return button;
}

Spinbox::Spinbox()
    : Widget()
{
}

Spinbox::~Spinbox()
{
}

bool Spinbox::parse(XMLElement *e, std::vector<std::string>& errors)
{
    if(!Widget::parse(e, errors)) return false;

    std::string tag(e->Value());
    if(tag != "spinbox")
    {
        errors.push_back("element must be <spinbox>");
        return false;
    }

    if(!e->Attribute("minimum") || e->QueryIntAttribute("minimum", &minimum) != XML_NO_ERROR)
        minimum = 0;

    if(!e->Attribute("maximum") || e->QueryIntAttribute("maximum", &maximum) != XML_NO_ERROR)
        maximum = 100;

    if(e->Attribute("prefix")) prefix = e->Attribute("prefix");
    else prefix = "";

    if(e->Attribute("suffix")) suffix = e->Attribute("suffix");
    else suffix = "";

    if(!e->Attribute("step") || e->QueryIntAttribute("step", &step) != XML_NO_ERROR)
        step = 1;

    if(e->Attribute("onchange")) onchange = e->Attribute("onchange");
    else onchange = "";

    return true;
}

QWidget * Spinbox::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QSpinBox *slider = new QSpinBox(parent);
    slider->setMinimum(minimum);
    slider->setMaximum(maximum);
    slider->setPrefix(QString::fromStdString(prefix));
    slider->setSuffix(QString::fromStdString(suffix));
    slider->setSingleStep(step);
    QObject::connect(slider, SIGNAL(valueChanged(int)), uiproxy, SLOT(onValueChange(int)));
    qwidget = slider;
    Widget::widgetByQWidget[qwidget] = this;
    this->proxy = proxy;
    return slider;
}

Group::Group()
    : Widget()
{
}

Group::~Group()
{
    for(std::vector< std::vector<Widget*> >::iterator it = children.begin(); it != children.end(); ++it)
    {
        for(std::vector<Widget*>::iterator it2 = it->begin(); it2 != it->end(); ++it2)
        {
            delete *it2;
        }
    }
}

bool Group::parse(XMLElement *e, std::vector<std::string>& errors)
{
    if(!Widget::parse(e, errors)) return false;

    std::string tag(e->Value());
    if(tag != "group")
    {
        errors.push_back("element must be <group>");
        return false;
    }

    if(e->Attribute("text")) text = e->Attribute("text");
    else text = "";

    return LayoutWidget::parse(e, errors);
}

QWidget * Group::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QGroupBox *groupBox = new QGroupBox(QString::fromStdString(text), parent);
    LayoutWidget::createQtWidget(proxy, uiproxy, groupBox);
    qwidget = groupBox;
    Widget::widgetByQWidget[qwidget] = this;
    this->proxy = proxy;
    return groupBox;
}

Window::Window()
    : qwidget(NULL),
      proxy(NULL)
{
}

Window::~Window()
{
#ifdef DEBUG
    std::cerr << "Window::~Window() - contents of Widget::widgets (BEFORE DTOR):" << std::endl;
    for(std::map<int, Widget*>::const_iterator it = Widget::widgets.begin(); it != Widget::widgets.end(); ++it)
        std::cerr << "    " << it->first << ": " << std::hex << ((void*)it->second) << std::dec << std::endl;
    std::cerr << "Window::~Window() - end" << std::endl;

    std::cerr << "Window::~Window() - contents of Widget::widgetByQWidget (BEFORE DTOR):" << std::endl;
    for(std::map<QWidget*, Widget*>::const_iterator it = Widget::widgetByQWidget.begin(); it != Widget::widgetByQWidget.end(); ++it)
        std::cerr << "    " << std::hex << it->first << std::dec << ": " << std::hex << ((void*)it->second) << std::dec << " (id=" << it->second->id << ")" << std::endl;
    std::cerr << "Window::~Window() - end" << std::endl;
#endif

#ifdef DEBUG
    std::cerr << "Window::~Window() - deleting 'children' items" << std::endl;
#endif

    for(std::vector< std::vector<Widget*> >::iterator it = children.begin(); it != children.end(); ++it)
    {
        for(std::vector<Widget*>::iterator it2 = it->begin(); it2 != it->end(); ++it2)
        {
            delete *it2;
        }
    }

    if(qwidget)
    {
#ifdef DEBUG
    std::cerr << "Window::~Window() - deleting member 'qwidget'" << std::endl;
#endif

        delete qwidget;
    }

#ifdef DEBUG
    std::cerr << "Window::~Window() - contents of Widget::widgets (AFTER DTOR):" << std::endl;
    for(std::map<int, Widget*>::const_iterator it = Widget::widgets.begin(); it != Widget::widgets.end(); ++it)
        std::cerr << "    " << it->first << ": " << std::hex << ((void*)it->second) << std::dec << std::endl;
    std::cerr << "Window::~Window() - end" << std::endl;

    std::cerr << "Window::~Window() - contents of Widget::widgetByQWidget (AFTER DTOR):" << std::endl;
    for(std::map<QWidget*, Widget*>::const_iterator it = Widget::widgetByQWidget.begin(); it != Widget::widgetByQWidget.end(); ++it)
        std::cerr << "    " << std::hex << it->first << std::dec << ": " << std::hex << ((void*)it->second) << std::dec << " (id=" << it->second->id << ")" << std::endl;
    std::cerr << "Window::~Window() - end" << std::endl;
#endif
}

bool Window::parse(XMLElement *e, std::vector<std::string>& errors)
{
    std::string tag(e->Value());
    if(tag != "ui")
    {
        errors.push_back("root element must be <ui>");
        return false;
    }

    if(e->Attribute("title")) title = e->Attribute("title");
    else title = "Custom UI";

    return LayoutWidget::parse(e, errors);
}

QWidget * Window::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QDialog *window = new QDialog(parent, Qt::Tool);
    LayoutWidget::createQtWidget(proxy, uiproxy, window);
    window->setWindowTitle(QString::fromStdString(title));
    window->setWindowFlags(Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
    //window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();
    qwidget = window;
    this->proxy = proxy;
    return window;
}

