#include "SVG.h"

#include "XMLUtils.h"

#include "UI.h"

#include <QMouseEvent>
#include <QLabel>

#include "stubs.h"

SVG::SVG()
    : Widget("svg")
{
}

SVG::~SVG()
{
}

void SVG::parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(parent, widgets, e);

    onMouseDown = xmlutils::getAttrStr(e, "on-mouse-down", "");

    onMouseUp = xmlutils::getAttrStr(e, "on-mouse-up", "");

    onMouseMove = xmlutils::getAttrStr(e, "on-mouse-move", "");

    file = xmlutils::getAttrStr(e, "file", "");

    auto svgdoc = e->FirstChildElement("svg");
    if(svgdoc != NULL)
        data = xmlutils::elementToString(svgdoc);
}

QWidget * SVG::createQtWidget(Proxy *proxy, UI *ui, QWidget *parent)
{
    SvgWidget *w = new SvgWidget(parent, this);
    w->setEnabled(enabled);
    w->setVisible(visible);
    w->setStyleSheet(QString::fromStdString(style));
    if(file != "")
    {
        w->load(QString::fromStdString(file));
    }
    if(data != "")
    {
        QByteArray qdata(data.data(), data.size());
        w->load(qdata);
    }
    QObject::connect(w, &SvgWidget::mouseEvent, ui, &UI::onMouseEvent);
    setQWidget(w);
    setProxy(proxy);
    return w;
}

void SVG::loadFile(const QString &file)
{
    SvgWidget *w = static_cast<SvgWidget*>(getQWidget());
    w->load(file);
}

void SVG::loadData(const QByteArray &contents)
{
    SvgWidget *w = static_cast<SvgWidget*>(getQWidget());
    w->load(contents);
}

SvgWidget::SvgWidget(QWidget *parent, SVG *svg_)
    : QSvgWidget(parent), svg(svg_)
{
    setMouseTracking(true);
}

void SvgWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(svg->onMouseMove != "")
    {
        event->accept();
        int type = simui_mouse_move;
        bool shift = event->modifiers() & Qt::ShiftModifier;
        bool control = event->modifiers() & Qt::ControlModifier;
        int x = event->x();
        int y = event->y();
        emit mouseEvent(svg, type, shift, control, x, y);
    }
    else
    {
        event->ignore();
    }
}

void SvgWidget::mousePressEvent(QMouseEvent *event)
{
    if(svg->onMouseDown != "" && event->button() == Qt::LeftButton)
    {
        event->accept();
        int type = simui_mouse_left_button_down;
        bool shift = event->modifiers() & Qt::ShiftModifier;
        bool control = event->modifiers() & Qt::ControlModifier;
        int x = event->x();
        int y = event->y();
        emit mouseEvent(svg, type, shift, control, x, y);
    }
    else
    {
        event->ignore();
    }
}

void SvgWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(svg->onMouseUp != "" && event->button() == Qt::LeftButton)
    {
        event->accept();
        int type = simui_mouse_left_button_up;
        bool shift = event->modifiers() & Qt::ShiftModifier;
        bool control = event->modifiers() & Qt::ControlModifier;
        int x = event->x();
        int y = event->y();
        emit mouseEvent(svg, type, shift, control, x, y);
    }
    else
    {
        event->ignore();
    }
}

