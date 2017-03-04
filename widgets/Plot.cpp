#include "Plot.h"

#include "XMLUtils.h"

#include "UIProxy.h"

#include "QCustomPlot.h"

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

