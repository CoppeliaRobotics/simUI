#include "Image.h"

#include "XMLUtils.h"

#include "UIProxy.h"

#include <QLabel>

Image::Image()
    : Widget("image")
{
}

Image::~Image()
{
}

void Image::parse(tinyxml2::XMLElement *e)
{
    Widget::parse(e);
}

QWidget * Image::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QLabel *label = new QLabel(parent);
    setQWidget(label);
    setProxy(proxy);
    return label;
}

