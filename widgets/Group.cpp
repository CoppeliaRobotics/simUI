#include "Group.h"

#include "XMLUtils.h"

#include "UI.h"

#include <QGroupBox>

Group::Group()
    : Widget("group")
{
}

Group::~Group()
{
}

void Group::parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(parent, widgets, e);

    flat = xmlutils::getAttrBool(e, "flat", false);

    LayoutWidget::parse(this, parent, widgets, e);
}

QWidget * Group::createQtWidget(Proxy *proxy, UI *ui, QWidget *parent)
{
    QWidget *groupBox = flat ?
        new QWidget(parent) :
        new QGroupBox(parent);
    if(flat)
        groupBox->setContentsMargins(0, 0, 0, 0);
    groupBox->setEnabled(enabled);
    groupBox->setVisible(visible);
    groupBox->setStyleSheet(QString::fromStdString(style));
    LayoutWidget::createQtWidget(proxy, ui, groupBox);
    setQWidget(groupBox);
    setProxy(proxy);
    return groupBox;
}

