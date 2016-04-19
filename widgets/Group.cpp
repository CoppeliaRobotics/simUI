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

void Group::parse(tinyxml2::XMLElement *e)
{
    Widget::parse(e);

    text = xmlutils::getAttrStr(e, "text", "");

    flat = xmlutils::getAttrBool(e, "flat", false);

    LayoutWidget::parse(e);
}

QWidget * Group::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QWidget *groupBox = flat ?
        new QWidget(parent) :
        new QGroupBox(QString::fromStdString(text), parent);
    LayoutWidget::createQtWidget(proxy, uiproxy, groupBox);
    setQWidget(groupBox);
    setProxy(proxy);
    return groupBox;
}

