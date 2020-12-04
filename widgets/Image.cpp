#include "Image.h"

#include "XMLUtils.h"

#include "UIProxy.h"

#include <QMouseEvent>
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

    scaledContents = xmlutils::getAttrBool(e, "scaled-contents", false);

    keepAspectRatio = xmlutils::getAttrBool(e, "keep-aspect-ratio", false);

    onMouseDown = xmlutils::getAttrStr(e, "on-mouse-down", "");

    onMouseUp = xmlutils::getAttrStr(e, "on-mouse-up", "");

    onMouseMove = xmlutils::getAttrStr(e, "on-mouse-move", "");

    file = xmlutils::getAttrStr(e, "file", "");
}

QWidget * Image::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QImageWidget *label = new QImageWidget(parent, this);
    label->setEnabled(enabled);
    label->setVisible(visible);
    label->setStyleSheet(QString::fromStdString(style));
    label->setScaledContents(scaledContents);
    if(width > 0 && height > 0)
    {
        QImage img(width, height, QImage::Format_ARGB32);
        img.fill(QColor(0,0,0,0).rgba());
        label->setPixmap(QPixmap::fromImage(img));
    }
    if(file != "")
    {
        UIProxy::getInstance()->loadImageFromFile(this, file.c_str(), width, height);
    }
    QObject::connect(label, &QImageWidget::mouseEvent, uiproxy, &UIProxy::onMouseEvent);
    setQWidget(label);
    setProxy(proxy);
    return label;
}

void Image::setImage(const char *data, int w, int h)
{
    if(!data) return;
    QImage::Format format = QImage::Format_RGB888;
    int bpp = 3; // bytes per pixel
    QPixmap pixmap = QPixmap::fromImage(QImage((unsigned char *)data, w, h, bpp * w, format));
    sim::releaseBuffer((char *)data); // XXX: simReleaseBuffer should accept a const pointer?
    QImageWidget *qimage = static_cast<QImageWidget*>(getQWidget());
    qimage->setPixmap(pixmap);
    qimage->resize(pixmap.size());
}

QImageWidget::QImageWidget(QWidget *parent, Image *image_)
    : QLabel(parent), image(image_)
{
    pixmapWidth = 0;
    pixmapHeight = 0;
    setMouseTracking(true);
}

void QImageWidget::setPixmap(const QPixmap &pm)
{
    pixmapWidth = pm.width();
    pixmapHeight = pm.height();
    updateMargins();
    QLabel::setPixmap(pm);
}

void QImageWidget::resizeEvent(QResizeEvent *event)
{
    updateMargins();
    QLabel::resizeEvent(event);
}

void QImageWidget::updateMargins()
{
    if(pixmapWidth <= 0 || pixmapHeight <= 0) return;

    if(!image->keepAspectRatio)
    {
        setContentsMargins(0, 0, 0, 0);
        return;
    }

    int w = this->width();
    int h = this->height();

    if(w <= 0 || h <= 0) return;
    if(w * pixmapHeight > h * pixmapWidth)
    {
        int m = (w - (pixmapWidth * h / pixmapHeight)) / 2;
        setContentsMargins(m, 0, m, 0);
    }
    else
    {
        int m = (h - (pixmapHeight * w / pixmapWidth)) / 2;
        setContentsMargins(0, m, 0, m);
    }
}

void QImageWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(image->onMouseMove != "")
    {
        event->accept();
        int type = sim_ui_mouse_move;
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
        int type = sim_ui_mouse_left_button_down;
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
        int type = sim_ui_mouse_left_button_up;
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

