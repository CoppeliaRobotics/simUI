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
    QObject::connect(label, &QImageWidget::mouseEvent, uiproxy, &UIProxy::onMouseEvent);
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
        int type = sim_customui_mouse_move;
        bool shift = event->modifiers() & Qt::ShiftModifier;
        bool control = event->modifiers() & Qt::ControlModifier;
        int x = event->x();
        int y = event->y();
        emit mouseEvent(image, type, shift, control, x, y);
    }
    else
    {
        event->ignore();
    }
}

void QImageWidget::mousePressEvent(QMouseEvent *event)
{
    if(image->onMouseDown != "" && event->button() == Qt::LeftButton)
    {
        event->accept();
        int type = sim_customui_mouse_left_button_down;
        bool shift = event->modifiers() & Qt::ShiftModifier;
        bool control = event->modifiers() & Qt::ControlModifier;
        int x = event->x();
        int y = event->y();
        emit mouseEvent(image, type, shift, control, x, y);
    }
    else
    {
        event->ignore();
    }
}

void QImageWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(image->onMouseUp != "" && event->button() == Qt::LeftButton)
    {
        event->accept();
        int type = sim_customui_mouse_left_button_up;
        bool shift = event->modifiers() & Qt::ShiftModifier;
        bool control = event->modifiers() & Qt::ControlModifier;
        int x = event->x();
        int y = event->y();
        emit mouseEvent(image, type, shift, control, x, y);
    }
    else
    {
        event->ignore();
    }
}

