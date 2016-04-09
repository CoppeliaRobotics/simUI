#include "Edit.h"

#include "UIProxy.h"

#include <QLineEdit>

Edit::Edit()
    : Widget()
{
}

Edit::~Edit()
{
}

bool Edit::parse(tinyxml2::XMLElement *e, std::vector<std::string>& errors)
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

