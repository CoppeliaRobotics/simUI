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

    onMouseDown = xmlutils::getAttrStr(e, "onmousedown", "");

    onMouseUp = xmlutils::getAttrStr(e, "onmouseup", "");

    onMouseMove = xmlutils::getAttrStr(e, "onmousemove", "");

    file = xmlutils::getAttrStr(e, "file", "");
}

QWidget * Image::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QImageWidget *label = new QImageWidget(parent, this);
    label->setEnabled(enabled);
    label->setVisible(visible);
    label->setStyleSheet(QString::fromStdString(style));
    if(file != "")
    {
        UIProxy::getInstance()->loadImageFromFile(this, file.c_str(), width, height);
    }
    if(onMouseDown != "")
    {
        QObject::connect(label, SIGNAL(mouseDown(Image*,QMouseEvent*)), uiproxy, SLOT(onMouseDown(Image*,QMouseEvent*)));
    }
    if(onMouseUp != "")
    {
        QObject::connect(label, SIGNAL(mouseUp(Image*,QMouseEvent*)), uiproxy, SLOT(onMouseUp(Image*,QMouseEvent*)));
    }
    if(onMouseMove != "")
    {
        QObject::connect(label, SIGNAL(mouseMove(Image*,QMouseEvent*)), uiproxy, SLOT(onMouseMove(Image*,QMouseEvent*)));
    }
    setQWidget(label);
    setProxy(proxy);
    return label;
}

QImageWidget::QImageWidget(QWidget *parent, Image *image_)
    : QLabel(parent), image(image_)
{
    setMouseTracking(true);
}

void QImageWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(image->onMouseMove != "")
    {
        event->accept();
        emit mouseMove(image, event);
    }
    else
    {
        event->ignore();
    }
}

void QImageWidget::mousePressEvent(QMouseEvent *event)
{
    if(image->onMouseDown != "")
    {
        event->accept();
        emit mouseDown(image, event);
    }
    else
    {
        event->ignore();
    }
}

void QImageWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(image->onMouseUp != "")
    {
        event->accept();
        emit mouseUp(image, event);
    }
    else
    {
        event->ignore();
    }
}

