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

    background_color = xmlutils::getAttrIntV(e, "background-color", "255,255,255", 3, 3, ",");

    graduation = xmlutils::getAttrBool(e, "graduation", false);

    graduation_color = xmlutils::getAttrIntV(e, "graduation-color", "0,0,0", 3, 3, ",");

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

QWidget * Plot::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QCustomPlot *plot = new QCustomPlot(parent);
    plot->setEnabled(enabled);
    plot->setVisible(visible);
    plot->setStyleSheet(QString::fromStdString(style));
    setQWidget(plot);
    setProxy(proxy);
    return plot;
}

void Plot::addCurve(std::string name, QCPGraph *curve)
{
    std::map<std::string, QCPGraph *>::iterator it = curveByName_.find(name);
    if(it != curveByName_.end())
    {
        std::stringstream ss;
        ss << "curve with name \"" << name << "\" already exists";
        throw std::runtime_error(ss.str());
    }
    curveByName_[name] = curve;
}

void Plot::removeCurve(std::string name)
{
    std::map<std::string, QCPGraph *>::iterator it = curveByName_.find(name);
    if(it == curveByName_.end())
    {
        std::stringstream ss;
        ss << "curve with name \"" << name << "\" does not exist";
        throw std::runtime_error(ss.str());
    }
    curveByName_.erase(it);
}

QCPGraph * Plot::curveByName(std::string name)
{
    std::map<std::string, QCPGraph *>::iterator it = curveByName_.find(name);
    if(it == curveByName_.end())
    {
        std::stringstream ss;
        ss << "curve with name \"" << name << "\" does not exist";
        throw std::runtime_error(ss.str());
    }
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

void Plot::trim(QCPGraph *curve)
{
    QSharedPointer<QCPGraphDataContainer> pdata = curve->data();
    if(max_buffer_size > 0 && pdata->size() > max_buffer_size)
    {
        double k = pdata->at(pdata->size() - max_buffer_size)->key;
        pdata->removeBefore(k);
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

