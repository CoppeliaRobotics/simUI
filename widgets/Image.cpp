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

void Image::parse(std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(widgets, e);

    width = xmlutils::getAttrInt(e, "width", -1);

    height = xmlutils::getAttrInt(e, "height", -1);

    file = xmlutils::getAttrStr(e, "file", "");
}

QWidget * Image::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QLabel *label = new QLabel(parent);
    if(file != "")
    {
        QImage::Format format = QImage::Format_RGB888;
        int bpp = 3; // bytes per pixel
        QPixmap pixmap;
        int resolution[2];
        simUChar *img = simLoadImage(resolution, 0, file.c_str(), NULL);
        simTransformImage(img, resolution, 4, NULL, NULL, NULL);
        if(width > 0 && height > 0)
        {
            int size[2] = {width, height};
            simUChar *scaledImg = simGetScaledImage(img, resolution, size, 0, NULL);
            pixmap = QPixmap::fromImage(QImage((unsigned char *)scaledImg, width, height, bpp * width, format));
            simReleaseBuffer((simChar *)scaledImg);
        }
        else
        {
            pixmap = QPixmap::fromImage(QImage((unsigned char *)img, resolution[0], resolution[1], bpp * resolution[0], format));
        }
        simReleaseBufferE((simChar *)img);
        label->setPixmap(pixmap);
        label->resize(pixmap.size());
    }
    setQWidget(label);
    setProxy(proxy);
    return label;
}

