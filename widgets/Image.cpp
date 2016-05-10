#include "Image.h"

#include "XMLUtils.h"

#include "UIProxy.h"

#include <QLabel>

#include "v_repLib.h"

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

    width = xmlutils::getAttrInt(e, "width", -1);

    height = xmlutils::getAttrInt(e, "height", -1);

    file = xmlutils::getAttrStr(e, "file", "");
}

QWidget * Image::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QLabel *label = new QLabel(parent);
    if(file != "")
    {
        QPixmap pixmap;
        int resolution[2];
        simUChar *img = simLoadImage(resolution, 0, file.c_str(), NULL);
        if(width > 0 && height > 0)
        {
            int size[2] = {width, height};
            simUChar *scaledImg = simGetScaledImage(img, resolution, size, 0, NULL);
            pixmap = QPixmap::fromImage(QImage((unsigned char *)scaledImg, width, height, 3*width, QImage::Format_RGB888));
            simReleaseBuffer((simChar *)scaledImg);
        }
        else
        {
            pixmap = QPixmap::fromImage(QImage((unsigned char *)img, resolution[0], resolution[1], 3*resolution[0], QImage::Format_RGB888));
        }
        simReleaseBuffer((simChar *)img);
        label->setPixmap(pixmap);
        label->resize(pixmap.size());
    }
    setQWidget(label);
    setProxy(proxy);
    return label;
}

