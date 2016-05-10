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

void Group::parse(std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(widgets, e);

    flat = xmlutils::getAttrBool(e, "flat", false);

    LayoutWidget::parse(widgets, e);
}

QWidget * Group::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QGroupBox *groupBox = new QGroupBox(parent);
    groupBox->setFlat(flat);
    LayoutWidget::createQtWidget(proxy, uiproxy, groupBox);
    setQWidget(groupBox);
    setProxy(proxy);
    return groupBox;
}

