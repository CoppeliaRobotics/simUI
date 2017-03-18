#include "Plot.h"

#include "XMLUtils.h"

#include "UIProxy.h"

#include <stdexcept>

Plot::Plot()
    : Widget("plot")
{
}

Plot::~Plot()
{
}

void Plot::parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(parent, widgets, e);

    background_color = xmlutils::getAttrIntV(e, "background-color", "-1,-1,-1", 3, 3, ",");

    type = xmlutils::getAttrStr(e, "type", "time");
    if(type == "time") ;
    else if(type == "xy") ;
    else throw std::range_error("the value for the 'type' attribute must be one of 'time', 'xy'");

    square = xmlutils::getAttrBool(e, "square", false);

    max_buffer_size = xmlutils::getAttrInt(e, "max-buffer-size", 1000);

    cyclic_buffer = xmlutils::getAttrBool(e, "cyclic-buffer", false);
}

class MyCustomPlot : public QCustomPlot
{
private:
    Plot *plot_;

public:
    MyCustomPlot(Plot *plot, QWidget *parent) : QCustomPlot(parent), plot_(plot)
    {
    }

    bool hasHeightForWidth() const
    {
        return plot_->square;
    }

    int heightForWidth(int w) const
    {
        return plot_->square ? w : -1;
    }

    void mouseDoubleClickEvent(QMouseEvent *event)
    {
        if(event->button() == Qt::LeftButton)
        {
            rescaleAxes();
            replot();
        }

        QCustomPlot::mouseDoubleClickEvent(event);
    }
};

QWidget * Plot::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QCustomPlot *plot = new MyCustomPlot(this, parent);
    plot->setEnabled(enabled);
    plot->setVisible(visible);
    plot->setStyleSheet(QString::fromStdString(style));
    plot->setMinimumSize(QSize(400,200));
    QColor bgcol(plot->palette().color(plot->backgroundRole()));
    if(background_color[0] >= 0 && background_color[1] >= 0 && background_color[2] >= 0)
        bgcol.setRgb(background_color[0], background_color[1], background_color[2]);
    plot->setBackground(QBrush(bgcol));
    setQWidget(plot);
    setProxy(proxy);
    return plot;
}

CurveMap::iterator Plot::findCurve(std::string name)
{
    return curveByName_.find(name);
}

CurveMap::iterator Plot::curveNameMustExist(std::string name)
{
    CurveMap::iterator it = findCurve(name);
    if(it == curveByName_.end())
    {
        std::stringstream ss;
        ss << "curve with name \"" << name << "\" does not exist";
        throw std::runtime_error(ss.str());
    }
    return it;
}

void Plot::curveNameMustNotExist(std::string name)
{
    CurveMap::iterator it = findCurve(name);
    if(it != curveByName_.end())
    {
        std::stringstream ss;
        ss << "curve with name \"" << name << "\" already exists";
        throw std::runtime_error(ss.str());
    }
}

void Plot::replot(bool queue)
{
    if(queue)
        qplot()->replot(QCustomPlot::rpQueuedReplot); // can help performance when doing a lot of replots
    else
        qplot()->replot();
}

void Plot::addCurve(int type, std::string name, std::vector<int> color, int style, curve_options *opts)
{
    curveNameMustNotExist(name);

    QCPAbstractPlottable *curve = 0L;

    switch(type)
    {
    case sim_customui_curve_type_time:
        curve = addTimeCurve(name, color, style, opts);
        break;
    case sim_customui_curve_type_xy:
        curve = addXYCurve(name, color, style, opts);
        break;
    default:
        return;
    }

    setCurveCommonOptions(curve, name, color, style, opts);
}

void Plot::setCurveCommonOptions(QCPAbstractPlottable *curve, std::string name, std::vector<int> color, int style, curve_options *opts)
{
    curveByName_[name] = curve;

    curve->setName(QString::fromStdString(name));

    QColor qcolor(color[0], color[1], color[2]);
    QPen qpen;
    qpen.setColor(qcolor);
    qpen.setWidth(opts->line_size);
    curve->setPen(qpen);
}

QCPGraph * Plot::addTimeCurve(std::string name, std::vector<int> color, int style, curve_options *opts)
{
    QCPGraph *curve = new QCPGraph(qplot()->xAxis, qplot()->yAxis);

    switch(style)
    {
    case sim_customui_curve_style_scatter:
        curve->setLineStyle(QCPGraph::lsNone);
        break;
    case sim_customui_curve_style_line:
        curve->setLineStyle(QCPGraph::lsLine);
        break;
    case sim_customui_curve_style_line_and_scatter:
        curve->setLineStyle(QCPGraph::lsLine);
        break;
    case sim_customui_curve_style_step_left:
        curve->setLineStyle(QCPGraph::lsStepLeft);
        break;
    case sim_customui_curve_style_step_center:
        curve->setLineStyle(QCPGraph::lsStepCenter);
        break;
    case sim_customui_curve_style_step_right:
        curve->setLineStyle(QCPGraph::lsStepRight);
        break;
    case sim_customui_curve_style_impulse:
        curve->setLineStyle(QCPGraph::lsImpulse);
        break;
    }

    if(style == sim_customui_curve_style_scatter || style == sim_customui_curve_style_line_and_scatter)
        curve->setScatterStyle(QCPScatterStyle(Plot::scatterShape(opts->scatter_shape), opts->scatter_size));

    return curve;
}

QCPCurve * Plot::addXYCurve(std::string name, std::vector<int> color, int style, curve_options *opts)
{
    QCPCurve *curve = new QCPCurve(qplot()->xAxis, qplot()->yAxis);

    switch(style)
    {
    case sim_customui_curve_style_scatter:
        curve->setLineStyle(QCPCurve::lsNone);
        break;
    case sim_customui_curve_style_line:
        curve->setLineStyle(QCPCurve::lsLine);
        break;
    case sim_customui_curve_style_line_and_scatter:
        curve->setLineStyle(QCPCurve::lsLine);
        break;
    }

    if(style == sim_customui_curve_style_scatter || style == sim_customui_curve_style_line_and_scatter)
        curve->setScatterStyle(QCPScatterStyle(Plot::scatterShape(opts->scatter_shape), opts->scatter_size));

    return curve;
}

void Plot::clearCurve(std::string name)
{
    QCPAbstractPlottable *curve = curveByName(name);

    if(QCPGraph *curve_t = dynamic_cast<QCPGraph*>(curve))
        curve_t->setData(QVector<double>(), QVector<double>(), true);
    else if(QCPCurve *curve_xy = dynamic_cast<QCPCurve*>(curve))
        curve_xy->setData(QVector<double>(), QVector<double>(), QVector<double>(), true);
}

void Plot::removeCurve(std::string name)
{
    QCPAbstractPlottable *curve = curveByName(name);
    curveByName_.erase(name);
    qplot()->removePlottable(curve);
}

QCPAbstractPlottable * Plot::curveByName(std::string name)
{
    CurveMap::iterator it = curveNameMustExist(name);
    return it->second;
}

QCPScatterStyle::ScatterShape Plot::scatterShape(int x)
{
    switch(x)
    {
    case sim_customui_curve_scatter_shape_none:
        return QCPScatterStyle::ssNone;
    case sim_customui_curve_scatter_shape_dot:
        return QCPScatterStyle::ssDot;
    case sim_customui_curve_scatter_shape_cross:
        return QCPScatterStyle::ssCross;
    case sim_customui_curve_scatter_shape_plus:
        return QCPScatterStyle::ssPlus;
    case sim_customui_curve_scatter_shape_circle:
        return QCPScatterStyle::ssCircle;
    case sim_customui_curve_scatter_shape_disc:
        return QCPScatterStyle::ssDisc;
    case sim_customui_curve_scatter_shape_square:
        return QCPScatterStyle::ssSquare;
    case sim_customui_curve_scatter_shape_diamond:
        return QCPScatterStyle::ssDiamond;
    case sim_customui_curve_scatter_shape_star:
        return QCPScatterStyle::ssStar;
    case sim_customui_curve_scatter_shape_triangle:
        return QCPScatterStyle::ssTriangle;
    case sim_customui_curve_scatter_shape_triangle_inverted:
        return QCPScatterStyle::ssTriangleInverted;
    case sim_customui_curve_scatter_shape_cross_square:
        return QCPScatterStyle::ssCrossSquare;
    case sim_customui_curve_scatter_shape_plus_square:
        return QCPScatterStyle::ssPlusSquare;
    case sim_customui_curve_scatter_shape_cross_circle:
        return QCPScatterStyle::ssCrossCircle;
    case sim_customui_curve_scatter_shape_plus_circle:
        return QCPScatterStyle::ssPlusCircle;
    case sim_customui_curve_scatter_shape_peace:
        return QCPScatterStyle::ssPeace;
    }
    return QCPScatterStyle::ssNone;
}

void Plot::addTimeData(std::string name, const std::vector<double>& x, const std::vector<double>& y)
{
    QCPGraph *curve = dynamic_cast<QCPGraph*>(curveByName(name));

    if(!cyclic_buffer && max_buffer_size > 0 && curve->dataCount() >= max_buffer_size) return;

    curve->addData(QVector<double>::fromStdVector(x), QVector<double>::fromStdVector(y));

    if(max_buffer_size > 0)
    {
        QSharedPointer<QCPGraphDataContainer> pdata = curve->data();
        if(cyclic_buffer && pdata->size() > max_buffer_size)
        {
            // remove previous samples for cyclic buffer
            double k = pdata->at(pdata->size() - max_buffer_size)->key;
            pdata->removeBefore(k);
        }
        if(!cyclic_buffer && pdata->size() > max_buffer_size)
        {
            // remove excess samples for non-cyclic buffer
            double k = pdata->at(max_buffer_size - 1)->key;
            pdata->removeAfter(k);
        }
    }
}

void Plot::addXYData(std::string name, const std::vector<double>& t, const std::vector<double>& x, const std::vector<double>& y)
{
    QCPCurve *curve = dynamic_cast<QCPCurve*>(curveByName(name));

    if(!cyclic_buffer && max_buffer_size > 0 && curve->dataCount() >= max_buffer_size) return;

    curve->addData(QVector<double>::fromStdVector(t), QVector<double>::fromStdVector(x), QVector<double>::fromStdVector(y));

    if(max_buffer_size > 0)
    {
        QSharedPointer<QCPCurveDataContainer> pdata = curve->data();
        if(cyclic_buffer && pdata->size() > max_buffer_size)
        {
            // remove previous samples for cyclic buffer
            double k = pdata->at(pdata->size() - max_buffer_size)->t;
            pdata->removeBefore(k);
        }
        if(!cyclic_buffer && pdata->size() > max_buffer_size)
        {
            // remove excess samples for non-cyclic buffer
            double k = pdata->at(max_buffer_size - 1)->t;
            pdata->removeAfter(k);
        }
    }
}

void Plot::setXRange(double min, double max)
{
    qplot()->xAxis->setRange(min, max);
}

void Plot::setYRange(double min, double max)
{
    qplot()->yAxis->setRange(min, max);
}

void Plot::setXLabel(std::string label)
{
    qplot()->xAxis->setLabel(QString::fromStdString(label));
}

void Plot::setYLabel(std::string label)
{
    qplot()->yAxis->setLabel(QString::fromStdString(label));
}

void Plot::rescaleAxes(std::string name, bool onlyEnlargeX, bool onlyEnlargeY)
{
    QCPAbstractPlottable *curve = curveByName(name);
    rescaleAxes(curve, onlyEnlargeX, onlyEnlargeY);
}

void Plot::rescaleAxes(QCPAbstractPlottable *curve, bool onlyEnlargeX, bool onlyEnlargeY)
{
    curve->rescaleKeyAxis(onlyEnlargeX);
    curve->rescaleValueAxis(onlyEnlargeY);
}

void Plot::rescaleAxesAll(bool onlyEnlargeX, bool onlyEnlargeY)
{
    for(int i = 0; i < qplot()->plottableCount(); ++i)
    {
        QCPAbstractPlottable *curve = qplot()->plottable(i);
        rescaleAxes(curve, onlyEnlargeX, onlyEnlargeY);
        onlyEnlargeX = true;
        onlyEnlargeY = true;
    }
}

void Plot::setMouseOptions(bool panX, bool panY, bool zoomX, bool zoomY)
{
    QCP::Interactions interactions = qplot()->interactions();
    interactions &= ~(QCP::iRangeDrag | QCP::iRangeZoom);
    if(panX || panY) interactions |= QCP::iRangeDrag;
    if(zoomX || zoomY) interactions |= QCP::iRangeZoom;
    qplot()->setInteractions(interactions);
    QCPAxisRect *ar = qplot()->axisRect(0);
    Qt::Orientations panning, zooming;
    if(panX) panning |= Qt::Horizontal;
    if(panY) panning |= Qt::Vertical;
    if(zoomX) zooming |= Qt::Horizontal;
    if(zoomY) zooming |= Qt::Vertical;
    ar->setRangeDrag(panning);
    ar->setRangeZoom(zooming);
    simFloat wheelZoomFactor;
    if(simGetFloatParameter(sim_floatparam_mouse_wheel_zoom_factor, &wheelZoomFactor) != -1)
        ar->setRangeZoomFactor(wheelZoomFactor, wheelZoomFactor);
}

void Plot::setLegendVisibility(bool visible)
{
    qplot()->legend->setVisible(visible);
}

