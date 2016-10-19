#include "Image.h"

#include "XMLUtils.h"

#include "UIProxy.h"

#include <QLabel>

#include "stubs.h"

Image::Image()
    : Widget("image")
{
}

Image::~Image()
{
}

void Image::parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(parent, widgets, e);

    width = xmlutils::getAttrInt(e, "width", -1);

    height = xmlutils::getAttrInt(e, "height", -1);

    file = xmlutils::getAttrStr(e, "file", "");
}

QWidget * Image::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QLabel *label = new QLabel(parent);
    label->setEnabled(enabled);
    label->setVisible(visible);
    label->setStyleSheet(QString::fromStdString(style));
    if(file != "")
    {
        UIProxy::getInstance()->loadImageFromFile(this, file.c_str(), width, height);
    }
    setQWidget(label);
    setProxy(proxy);
    return label;
}

