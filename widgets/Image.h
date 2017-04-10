#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED

#include <vector>
#include <string>

#include <QLabel>

#include "tinyxml2.h"

class Proxy;
class UIProxy;

#include "Widget.h"

class Image : public Widget
{
protected:
    int width;
    int height;
    std::string file;
    std::string onMouseDown;
    std::string onMouseUp;
    std::string onMouseMove;

public:
    Image();
    virtual ~Image();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class UIFunctions;
    friend class QImageWidget;
};

class QImageWidget : public QLabel
{
    Q_OBJECT
private:
    Image *image;

public:
    QImageWidget(QWidget *parent, Image *image_);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

signals:
    void mouseEvent(Image *image, int type, bool shift, bool control, int x, int y);
};

#endif // IMAGE_H_INCLUDED

