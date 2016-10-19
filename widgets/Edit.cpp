#include "Edit.h"

#include "XMLUtils.h"

#include "UIProxy.h"

#include <QLineEdit>

Edit::Edit()
    : Widget("edit")
{
}

Edit::~Edit()
{
}

void Edit::parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(parent, widgets, e);

    value = xmlutils::getAttrStr(e, "value", "");

    onchange = xmlutils::getAttrStr(e, "onchange", "");

    oneditingfinished = xmlutils::getAttrStr(e, "oneditingfinished", "");
}

QWidget * Edit::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QLineEdit *edit = new QLineEdit(parent);
    edit->setEnabled(enabled);
    edit->setVisible(visible);
    edit->setStyleSheet(QString::fromStdString(style));
    edit->setText(QString::fromStdString(value));
    QObject::connect(edit, SIGNAL(textChanged(QString)), uiproxy, SLOT(onValueChange(QString)));
    QObject::connect(edit, SIGNAL(editingFinished()), uiproxy, SLOT(onEditingFinished()));
    setQWidget(edit);
    setProxy(proxy);
    return edit;
}

