#include "Edit.h"

#include "XMLUtils.h"

#include "UI.h"

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

    eval = xmlutils::getAttrBool(e, "evaluate-expressions", false);
}

QWidget * Edit::createQtWidget(Proxy *proxy, UI *ui, QWidget *parent)
{
    QLineEdit *edit = new QLineEdit(parent);
    edit->setEnabled(enabled);
    edit->setVisible(visible);
    edit->setStyleSheet(QString::fromStdString(style));
    edit->setText(QString::fromStdString(value));
    edit->setEchoMode(password ? QLineEdit::Password : QLineEdit::Normal);
    if(eval)
    {
        QObject::connect(edit, &QLineEdit::editingFinished, [=] {
            ui->evaluateExpression(this, edit->text());
        });
    }
    else
    {
        QObject::connect(edit, &QLineEdit::editingFinished, ui, &UI::onEditingFinished);
    }
    QObject::connect(edit, &QLineEdit::textChanged, ui, &UI::onValueChangeString);
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

void Edit::setEvaluationResult(QString value)
{
    if(!eval) return;
    QLineEdit *qedit = static_cast<QLineEdit*>(getQWidget());
    bool oldSignalsState = qedit->blockSignals(true);
    qedit->setText(value);
    qedit->blockSignals(oldSignalsState);
    UI::getInstance()->valueChangeString(this, value);
    UI::getInstance()->editingFinished(this, value);
}
