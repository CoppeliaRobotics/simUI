#include "Group.h"

#include "XMLUtils.h"

#include "UIProxy.h"

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

QWidget * Group::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QGroupBox *groupBox = new QGroupBox(parent);
    groupBox->setStyleSheet(QString::fromStdString(style));
    groupBox->setFlat(flat);
    LayoutWidget::createQtWidget(proxy, uiproxy, groupBox);
    setQWidget(groupBox);
    setProxy(proxy);
    return groupBox;
}

