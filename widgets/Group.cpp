#include "Group.h"

#include "XMLUtils.h"

#include "UIProxy.h"

#include <QGroupBox>

Group::Group()
    : Widget()
{
}

Group::~Group()
{
}

const char * Group::name()
{
    return "group";
}

void Group::parse(tinyxml2::XMLElement *e)
{
    Widget::parse(e);

    text = xmlutils::getAttrStr(e, "text", "");

    LayoutWidget::parse(e);
}

QWidget * Group::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QGroupBox *groupBox = new QGroupBox(QString::fromStdString(text), parent);
    LayoutWidget::createQtWidget(proxy, uiproxy, groupBox);
    setQWidget(groupBox);
    setProxy(proxy);
    return groupBox;
}

