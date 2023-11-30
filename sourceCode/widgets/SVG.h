#ifndef SVG_H_INCLUDED
#define SVG_H_INCLUDED

#include "config.h"

#include <vector>
#include <string>

#include <QSvgWidget>

#include "tinyxml2.h"

class Proxy;
class UI;

#include "Widget.h"
#include "Event.h"

class SVG : public Widget, public EventOnMouseDown, public EventOnMouseUp, public EventOnMouseMove
{
protected:
    std::string file;
    std::string data;

public:
    SVG();
    virtual ~SVG();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UI *ui, QWidget *parent);

    void loadFile(const QString &file);
    void loadData(const QByteArray &contents);

    friend class SIM;
    friend class SvgWidget;
};

class SvgWidget : public QSvgWidget
{
    Q_OBJECT
private:
    SVG *svg;

public:
    SvgWidget(QWidget *parent, SVG *svg_);
protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void mouseEvent(SVG *svg, int type, bool shift, bool control, int x, int y);
};

#endif // SVG_H_INCLUDED

