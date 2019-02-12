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

    password = xmlutils::getAttrBool(e, "password", false);

    onchange = xmlutils::getAttrStr(e, "on-change", "");

    oneditingfinished = xmlutils::getAttrStr(e, "on-editing-finished", "");
}

QWidget * Edit::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QLineEdit *edit = new QLineEdit(parent);
    edit->setEnabled(enabled);
    edit->setVisible(visible);
    edit->setStyleSheet(QString::fromStdString(style));
    edit->setText(QString::fromStdString(value));
    edit->setEchoMode(password ? QLineEdit::Password : QLineEdit::Normal);
    QObject::connect(edit, &QLineEdit::textChanged, uiproxy, &UIProxy::onValueChangeString);
    QObject::connect(edit, &QLineEdit::editingFinished, uiproxy, &UIProxy::onEditingFinished);
    setQWidget(edit);
    setProxy(proxy);
    return edit;
}

void Edit::setValue(std::string value, bool suppressSignals)
{
    QLineEdit *qedit = static_cast<QLineEdit*>(getQWidget());
    bool oldSignalsState = qedit->blockSignals(suppressSignals);
    qedit->setText(QString::fromStdString(value));
    qedit->blockSignals(oldSignalsState);
}

std::string Edit::getValue()
{
    QLineEdit *qedit = static_cast<QLineEdit*>(getQWidget());
    return qedit->text().toStdString();
}

