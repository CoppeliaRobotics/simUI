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

    zoomable = xmlutils::getAttrBool(e, "zoomable", false);

    simulation = xmlutils::getAttrBool(e, "simulation", false);
}

class MyCustomPlot : public QCustomPlot
{
private:
    Plot *plot_;

public:
    MyCustomPlot(Plot *plot, QWidget *parent) : plot_(plot), QCustomPlot(parent)
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
    if(zoomable)
        plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    setQWidget(plot);
    setProxy(proxy);
    return plot;
}

std::map<std::string, QCPGraph *>::iterator Plot::findCurve(std::string name)
{
    return curveByName_.find(name);
}

std::map<std::string, QCPGraph *>::iterator Plot::curveNameMustExist(std::string name)
{
    std::map<std::string, QCPGraph *>::iterator it = findCurve(name);
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
    std::map<std::string, QCPGraph *>::iterator it = findCurve(name);
    if(it != curveByName_.end())
    {
        std::stringstream ss;
        ss << "curve with name \"" << name << "\" already exists";
        throw std::runtime_error(ss.str());
    }
}

void Plot::replot()
{
    QCustomPlot *qplot = static_cast<QCustomPlot*>(getQWidget());
    qplot->replot();
}

void Plot::addCurve(std::string name, std::vector<int> color, int style, curve_options *opts)
{
    curveNameMustNotExist(name);

    QCustomPlot *qplot = static_cast<QCustomPlot*>(getQWidget());
    QCPGraph *curve = qplot->addGraph();

    curveByName_[name] = curve;

    curve->setName(QString::fromStdString(name));
    QColor qcolor(color[0], color[1], color[2]);
    curve->setPen(QPen(qcolor));

    if(qplot->graphCount() > 1)
        qplot->legend->setVisible(true);

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
}

void Plot::clearCurve(std::string name)
{
    QCPGraph *curve = curveByName(name);
    curve->setData(QVector<double>(), QVector<double>(), true);
}

void Plot::removeCurve(std::string name)
{
    QCPGraph *curve = curveByName(name);
    curveByName_.erase(name);

    QCustomPlot *qplot = static_cast<QCustomPlot*>(getQWidget());
    qplot->removeGraph(curve);
}

QCPGraph * Plot::curveByName(std::string name)
{
    std::map<std::string, QCPGraph *>::iterator it = curveNameMustExist(name);
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

void Plot::addData(std::string name, const std::vector<double>& x, const std::vector<double>& y)
{
    QCPGraph *curve = curveByName(name);
    if(!cyclic_buffer && max_buffer_size > 0 && curve->dataCount() >= max_buffer_size) return;
    curve->addData(QVector<double>::fromStdVector(x), QVector<double>::fromStdVector(y));
    trim(curve);
}

void Plot::trim(QCPGraph *curve)
{
    QSharedPointer<QCPGraphDataContainer> pdata = curve->data();

    // remove previous samples for cyclic buffer
    if(max_buffer_size > 0 && cyclic_buffer && pdata->size() > max_buffer_size)
    {
        double k = pdata->at(pdata->size() - max_buffer_size)->key;
        pdata->removeBefore(k);
    }

    // remove excess samples for non-cyclic buffer
    if(max_buffer_size > 0 && !cyclic_buffer && pdata->size() > max_buffer_size)
    {
        double k = pdata->at(max_buffer_size - 1)->key;
        pdata->removeAfter(k);
    }
}

void Plot::trim()
{
    QCustomPlot *qplot = static_cast<QCustomPlot*>(getQWidget());
    for(int i = 0; i < qplot->plottableCount(); ++i)
    {
        QCPGraph *curve = dynamic_cast<QCPGraph*>(qplot->plottable(i));
        if(curve)
            trim(curve);
    }
}

void Plot::rescale(QCPAbstractPlottable *curve, bool onlyEnlargeX, bool onlyEnlargeY)
{
    curve->rescaleKeyAxis(onlyEnlargeX);
    curve->rescaleValueAxis(onlyEnlargeY);
}

void Plot::rescale(bool onlyEnlargeX, bool onlyEnlargeY)
{
    QCustomPlot *qplot = static_cast<QCustomPlot*>(getQWidget());
    for(int i = 0; i < qplot->plottableCount(); ++i)
    {
        QCPAbstractPlottable *curve = dynamic_cast<QCPAbstractPlottable*>(qplot->plottable(i));
        if(curve)
        {
            rescale(curve, onlyEnlargeX, onlyEnlargeY);
            onlyEnlargeX = true;
            onlyEnlargeY = true;
        }
    }
}

