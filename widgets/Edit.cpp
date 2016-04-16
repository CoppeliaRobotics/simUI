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

const char * Edit::name()
{
    return "edit";
}

bool Edit::parse(tinyxml2::XMLElement *e, std::vector<std::string>& errors)
{
    if(!Widget::parse(e, errors)) return false;

    if(e->Attribute("value")) value = e->Attribute("value");
    else value = "";

    if(e->Attribute("onchange")) onchange = e->Attribute("onchange");
    else onchange = "";

    return true;
}

QWidget * Edit::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QLineEdit *edit = new QLineEdit(parent);
    edit->setText(QString::fromStdString(value));
    QObject::connect(edit, SIGNAL(textChanged(QString)), uiproxy, SLOT(onValueChange(QString)));
    qwidget = edit;
    Widget::widgetByQWidget[qwidget] = this;
    this->proxy = proxy;
    return edit;
}

