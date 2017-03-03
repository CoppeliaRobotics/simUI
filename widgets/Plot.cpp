#include "Plot.h"

#include "XMLUtils.h"

#include "UIProxy.h"

#include "QCustomPlot.h"

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

    bool graduation = xmlutils::getAttrBool(e, "graduation", false);

    graduation_color = xmlutils::getAttrIntV(e, "graduation-color", "0,0,0", 3, 3, ",");

    std::string type = xmlutils::getAttrStr(e, "type", "time");
    if(type == "time") ;
    else if(type == "xy") ;
    else throw std::range_error("the value for the 'type' attribute must be one of 'time', 'xy'");

    bool square = xmlutils::getAttrBool(e, "square", false);

    int max_buffer_size = xmlutils::getAttrInt(e, "max-buffer-size", 1000);

    bool cyclic_buffer = xmlutils::getAttrBool(e, "cyclic-buffer", false);

    bool zoomable = xmlutils::getAttrBool(e, "zoomable", false);

    bool simulation = xmlutils::getAttrBool(e, "simulation", false);
}

QWidget * Plot::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QCustomPlot *plot = new QCustomPlot(parent);
    plot->setEnabled(enabled);
    plot->setVisible(visible);
    plot->setStyleSheet(QString::fromStdString(style));
    plot->addGraph();
    QVector<double> x(101), y(101); // initialize with entries 0..100
    for(int i = 0; i < 101; ++i)
    {
        x[i] = i / 50.0 - 1;
        y[i] = x[i] * x[i];
    }
    plot->graph(0)->setData(x, y);
    plot->xAxis->setLabel("x");
    plot->yAxis->setLabel("y");
    plot->xAxis->setRange(-1, 1);
    plot->yAxis->setRange(0, 1);
    plot->replot();
    setQWidget(plot);
    setProxy(proxy);
    return plot;
}

