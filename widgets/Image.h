#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED

#include "config.h"

#include <vector>
#include <string>

#include <QLabel>

#include "tinyxml2.h"

class Proxy;
class UIProxy;

#include "Widget.h"
#include "Event.h"

class Image : public Widget, public EventOnMouseDown, public EventOnMouseUp, public EventOnMouseMove
{
protected:
    int width;
    int height;
    std::string file;
    bool scaledContents;
    bool keepAspectRatio;

public:
    Image();
    virtual ~Image();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    void setImage(const char *data, int w, int h);

    friend class UIFunctions;
    friend class QImageWidget;
};

class QImageWidget : public QLabel
{
    Q_OBJECT
private:
    Image *image;
    int pixmapWidth;
    int pixmapHeight;

public:
    QImageWidget(QWidget *parent, Image *image_);
    void setPixmap(const QPixmap &pm);
protected:
    void resizeEvent(QResizeEvent *event) override;
    void updateMargins();
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void mouseEvent(Image *image, int type, bool shift, bool control, int x, int y);
};

#endif // IMAGE_H_INCLUDED

