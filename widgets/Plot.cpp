#include "Plot.h"

#include "XMLUtils.h"

#include "UIProxy.h"
#include "debug.h"

#include <stdexcept>

#include <boost/foreach.hpp>

#define SELECTED_SCATTER_MULT 2.0

Plot::Plot()
    : Widget("plot")
{
}

Plot::~Plot()
{
}

static bool isValidColor(const std::vector<int>& c)
{
    for(int i = 0; i < 3; i++)
        if(c[i] < 0 || c[i] > 255)
            return false;
    return true;
}

static QColor toQColor(const std::vector<int>& c)
{
    return QColor(c[0], c[1], c[2]);
}

void Plot::parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(parent, widgets, e);

    background_color = xmlutils::getAttrIntV(e, "background-color", "-1,-1,-1", 3, 3, ",");

    std::vector<int> foreground_color = xmlutils::getAttrIntV(e, "foreground-color", "-1,-1,-1", 3, 3, ",");

    std::vector<int> axis_color = xmlutils::getAttrIntV(e, "axis-color", foreground_color, 3, 3, ",");
    axis_x_color = xmlutils::getAttrIntV(e, "axis-x-color", axis_color, 3, 3, ",");
    axis_y_color = xmlutils::getAttrIntV(e, "axis-y-color", axis_color, 3, 3, ",");

    std::vector<int> label_color = xmlutils::getAttrIntV(e, "label-color", foreground_color, 3, 3, ",");
    label_x_color = xmlutils::getAttrIntV(e, "label-x-color", label_color, 3, 3, ",");
    label_y_color = xmlutils::getAttrIntV(e, "label-y-color", label_color, 3, 3, ",");

    std::vector<int> grid_color = xmlutils::getAttrIntV(e, "grid-color", foreground_color, 3, 3, ",");
    grid_x_color = xmlutils::getAttrIntV(e, "grid-x-color", grid_color, 3, 3, ",");
    grid_y_color = xmlutils::getAttrIntV(e, "grid-y-color", grid_color, 3, 3, ",");

    std::vector<int> tick_label_color = xmlutils::getAttrIntV(e, "tick-label-color", foreground_color, 3, 3, ",");
    tick_label_x_color = xmlutils::getAttrIntV(e, "tick-label-x-color", tick_label_color, 3, 3, ",");
    tick_label_y_color = xmlutils::getAttrIntV(e, "tick-label-y-color", tick_label_color, 3, 3, ",");

    type = xmlutils::getAttrStr(e, "type", "time");
    if(type == "time") ;
    else if(type == "xy") ;
    else throw std::range_error("the value for the 'type' attribute must be one of 'time', 'xy'");

    square = xmlutils::getAttrBool(e, "square", false);

    max_buffer_size = xmlutils::getAttrInt(e, "max-buffer-size", 1000);

    cyclic_buffer = xmlutils::getAttrBool(e, "cyclic-buffer", false);

    onCurveClick = xmlutils::getAttrStr(e, "onclick", "");

    onLegendClick = xmlutils::getAttrStr(e, "onlegendclick", "");

    bool ticks = xmlutils::getAttrBool(e, "ticks", true);
    x_ticks = xmlutils::getAttrBool(e, "x-ticks", ticks);
    y_ticks = xmlutils::getAttrBool(e, "y-ticks", ticks);

    bool tick_labels = xmlutils::getAttrBool(e, "tick-labels", true);
    x_tick_labels = xmlutils::getAttrBool(e, "x-tick-labels", tick_labels);
    y_tick_labels = xmlutils::getAttrBool(e, "y-tick-labels", tick_labels);
}

QWidget * Plot::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QCustomPlot *plot = new MyCustomPlot(this, parent);
    plot->setEnabled(enabled);
    plot->setVisible(visible);
    plot->setStyleSheet(QString::fromStdString(style));
    plot->setMinimumSize(QSize(square ? 200 : 400, 200));
    QColor bgcol(plot->palette().color(plot->backgroundRole()));
    if(isValidColor(background_color))
        bgcol = toQColor(background_color);
    plot->setBackground(QBrush(bgcol));
    plot->setInteraction(QCP::iSelectPlottables);
    if(onLegendClick != "")
        plot->setInteraction(QCP::iSelectLegend);
    plot->legend->setSelectableParts(QCPLegend::spItems);
    plot->setAutoAddPlottableToLegend(false);
    if(isValidColor(grid_x_color))
    {
        QPen pen = plot->xAxis->grid()->pen();
        pen.setColor(toQColor(grid_x_color));
        plot->xAxis->grid()->setPen(pen);
    }
    if(isValidColor(grid_y_color))
    {
        QPen pen = plot->yAxis->grid()->pen();
        pen.setColor(toQColor(grid_y_color));
        plot->yAxis->grid()->setPen(pen);
    }
    if(isValidColor(axis_x_color))
    {
        QPen basePen = plot->xAxis->basePen();
        basePen.setColor(toQColor(axis_x_color));
        plot->xAxis->setBasePen(basePen);
        QPen tickPen = plot->xAxis->tickPen();
        tickPen.setColor(toQColor(axis_x_color));
        plot->xAxis->setTickPen(tickPen);
        QPen subTickPen = plot->xAxis->subTickPen();
        subTickPen.setColor(toQColor(axis_x_color));
        plot->xAxis->setSubTickPen(subTickPen);
    }
    if(isValidColor(axis_y_color))
    {
        QPen basePen = plot->yAxis->basePen();
        basePen.setColor(toQColor(axis_y_color));
        plot->yAxis->setBasePen(basePen);
        QPen tickPen = plot->yAxis->tickPen();
        tickPen.setColor(toQColor(axis_y_color));
        plot->yAxis->setTickPen(tickPen);
        QPen subTickPen = plot->yAxis->subTickPen();
        subTickPen.setColor(toQColor(axis_y_color));
        plot->yAxis->setSubTickPen(subTickPen);
    }
    if(isValidColor(label_x_color))
    {
        plot->xAxis->setLabelColor(toQColor(label_x_color));
    }
    if(isValidColor(label_y_color))
    {
        plot->yAxis->setLabelColor(toQColor(label_y_color));
    }
    if(isValidColor(tick_label_x_color))
    {
        plot->xAxis->setTickLabelColor(toQColor(tick_label_x_color));
    }
    if(isValidColor(tick_label_y_color))
    {
        plot->yAxis->setTickLabelColor(toQColor(tick_label_y_color));
    }
    plot->legend->setBrush(QBrush(bgcol));
    if(isValidColor(label_y_color))
    {
        plot->legend->setTextColor(toQColor(label_y_color));
    }
    if(isValidColor(grid_x_color))
    {
        QPen pen = plot->legend->borderPen();
        pen.setColor(toQColor(grid_x_color));
        plot->legend->setBorderPen(pen);
        plot->legend->setSelectedTextColor(toQColor(grid_x_color));
        plot->legend->setSelectedIconBorderPen(pen);
    }
    plot->xAxis->setTicks(x_ticks);
    plot->yAxis->setTicks(y_ticks);
    plot->xAxis->setTickLabels(x_tick_labels);
    plot->yAxis->setTickLabels(y_tick_labels);
    plot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QObject::connect(plot, SIGNAL(plottableClick(QCPAbstractPlottable*,int,QMouseEvent*)), uiproxy, SLOT(onPlottableClick(QCPAbstractPlottable*,int,QMouseEvent*)));
    QObject::connect(plot, SIGNAL(legendClick(QCPLegend*,QCPAbstractLegendItem*,QMouseEvent*)), uiproxy, SLOT(onLegendClick(QCPLegend*,QCPAbstractLegendItem*,QMouseEvent*)));
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

QCPGraph * Plot::curveMustBeTime(QCPAbstractPlottable *curve)
{
    QCPGraph *ret = dynamic_cast<QCPGraph*>(curve);
    if(!ret)
        throw std::runtime_error("curve must be of type 'time'");
    return ret;
}

QCPCurve * Plot::curveMustBeXY(QCPAbstractPlottable *curve)
{
    QCPCurve *ret = dynamic_cast<QCPCurve*>(curve);
    if(!ret)
        throw std::runtime_error("curve must be of type 'xy'");
    return ret;
}

void Plot::replot(bool queue)
{
    if(square)
        squareRanges();

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

    QPen qpen;
    qpen.setColor(toQColor(color));
    qpen.setWidth(opts->line_size);
    switch(opts->line_style)
    {
    case sim_customui_line_style_solid:
        qpen.setStyle(Qt::SolidLine);
        break;
    case sim_customui_line_style_dashed:
        qpen.setStyle(Qt::DashLine);
        break;
    case sim_customui_line_style_dotted:
        qpen.setStyle(Qt::DotLine);
        break;
    }
    curve->setPen(qpen);

    if(opts->selectable)
        curve->setSelectable(QCP::stSingleData);

    if(opts->add_to_legend)
        curve->addToLegend(qplot()->legend);
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

    curve->selectionDecorator()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, toQColor(color), opts->scatter_size * SELECTED_SCATTER_MULT), QCPScatterStyle::spAll);

    if(opts->track)
    {
        tracers[curve] = new Tracer(qplot(), curve, this, opts);
    }

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

    curve->selectionDecorator()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, toQColor(color), opts->scatter_size * SELECTED_SCATTER_MULT), QCPScatterStyle::spAll);

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

    // remove tracer if any:
    if(QCPGraph *graph = dynamic_cast<QCPGraph*>(curve))
    {
        std::map<QCPGraph*, Tracer*>::iterator it = tracers.find(graph);
        if(it != tracers.end())
        {
            delete it->second;
            tracers.erase(it);
        }
    }

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
    QCPGraph *curve = curveMustBeTime(curveByName(name));

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
    QCPCurve *curve = curveMustBeXY(curveByName(name));

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

void Plot::getCurveData(std::string name, std::vector<double>& t, std::vector<double>& x, std::vector<double>& y)
{
    QCPAbstractPlottable *plottable = curveByName(name);

    t.clear();
    x.clear();
    y.clear();

    if(QCPGraph *graph = dynamic_cast<QCPGraph*>(plottable))
    {
	QSharedPointer<QCPGraphDataContainer> data = graph->data();
	for(int i = 0; i < data->size(); ++i)
	{
	    QCPGraphDataContainer::const_iterator dataItem = data->at(i);
	    x.push_back(dataItem->key);
	    y.push_back(dataItem->value);
	}
    }
    else if(QCPCurve *curve = dynamic_cast<QCPCurve*>(plottable))
    {
	QSharedPointer<QCPCurveDataContainer> data = curve->data();
	for(int i = 0; i < data->size(); ++i)
	{
	    QCPCurveDataContainer::const_iterator dataItem = data->at(i);
	    t.push_back(dataItem->t);
	    x.push_back(dataItem->key);
	    y.push_back(dataItem->value);
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

void Plot::squareRanges()
{
    double axesRatio = qplot()->xAxis->range().size() / qplot()->yAxis->range().size();
    double plotRatio = double(qplot()->size().width()) / double(qplot()->size().height());
    double ratio = axesRatio / plotRatio;
    if(axesRatio > plotRatio)
    {
        double r = qplot()->yAxis->range().size();
        double c = qplot()->yAxis->range().center();
        qplot()->yAxis->setRange(c - r * 0.5 * ratio, c + r * 0.5 * ratio);
    }
    else
    {
        double r = qplot()->xAxis->range().size();
        double c = qplot()->xAxis->range().center();
        qplot()->xAxis->setRange(c - r * 0.5 / ratio, c + r * 0.5 / ratio);
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
    simFloat wheelZoomFactor = UIProxy::wheelZoomFactor;
    ar->setRangeZoomFactor(pow(0.85, -wheelZoomFactor), pow(0.85, -wheelZoomFactor));
}

void Plot::setLegendVisibility(bool visible)
{
    qplot()->legend->setVisible(visible);
}

Tracer::Tracer(QCustomPlot *qplot_, QCPGraph *curve_, Plot *plot_, curve_options *opts_)
    : qplot(qplot_), curve(curve_), plot(plot_), opts(opts_)
{
    itemTracer = new QCPItemTracer(qplot);
    itemTracer->setGraph(curve);
    itemTracer->setInterpolating(false);
    //itemTracer->setPen(QPen(Qt::red));
    //itemTracer->setBrush(Qt::red);
    itemTracer->setSize(7);
    itemTracer->setStyle(QCPItemTracer::tsSquare);
    itemTracer->setVisible(false);

    itemTracerLabel = new QCPItemText(qplot);
    itemTracerLabel->setBrush(Qt::white);
    itemTracerLabel->setPositionAlignment(Qt::AlignLeft | Qt::AlignBottom);
    itemTracerLabel->setPadding(QMargins(8,8,8,8));
    itemTracerLabel->setVisible(false);
    itemTracerLabel->setLayer("axes");
}

void Tracer::trace(const QPoint& p)
{
    double positionX = qplot->xAxis->pixelToCoord(p.x());

    for(int i = curve->findBegin(positionX); i < curve->findEnd(positionX); i++)
    {
        double dist = curve->selectTest(p, false);
        if(dist >= 0.0 && dist <= 5.0)
        {
            itemTracer->setGraphKey(positionX);
            itemTracer->setVisible(true);

            double graphY = curve->dataMainValue(i);
            QString txt;
            txt.sprintf("%f, %f", positionX, graphY);
            itemTracerLabel->setText(txt);
            itemTracerLabel->position->setCoords(positionX, graphY);
            itemTracerLabel->setVisible(true);
        }
        else
        {
            itemTracer->setVisible(false);
            itemTracerLabel->setVisible(false);
        }
        qplot->replot();
        break;
    }
}

MyCustomPlot::MyCustomPlot(Plot *plot, QWidget *parent) : QCustomPlot(parent), plot_(plot)
{
    QObject::connect(this, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(onMousePress(QMouseEvent*)));
    QObject::connect(this, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(onMouseMove(QMouseEvent*)));
}

void MyCustomPlot::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        rescaleAxes();
        plot_->replot(false);
    }

    QCustomPlot::mouseDoubleClickEvent(event);
}

void MyCustomPlot::resizeEvent(QResizeEvent *event)
{
    QCustomPlot::resizeEvent(event);
    if(plot_->square)
        plot_->replot(false);
}

void MyCustomPlot::onMousePress(QMouseEvent *event)
{
    Qt::KeyboardModifiers mods = event->modifiers();
    if(mods == Qt::ShiftModifier)
        this->setSelectionRectMode(QCP::srmZoom);
    else if(mods == Qt::NoModifier)
        this->setSelectionRectMode(QCP::srmNone);
}

void MyCustomPlot::onMouseMove(QMouseEvent *event)
{
    typedef std::map<QCPGraph*, Tracer*> map_type;
    BOOST_FOREACH(map_type::value_type &i, plot_->tracers)
    {
        i.second->trace(event->pos());
    }
}

