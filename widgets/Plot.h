#ifndef PLOT_H_INCLUDED
#define PLOT_H_INCLUDED

#include <vector>
#include <map>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UIProxy;

#include "Widget.h"

#include "qcustomplot.h"

struct curve_options;

class Plot : public Widget
{
protected:
    std::vector<int> background_color;
    bool graduation;
    std::vector<int> graduation_color;
    std::string type;
    bool square;
    int max_buffer_size;
    bool cyclic_buffer;
    bool zoomable;
    bool simulation;

    std::map<std::string, QCPGraph *> curveByName_;

public:
    Plot();
    virtual ~Plot();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    void replot();

    void addCurve(std::string name, std::vector<int> color, int style, curve_options *opts);
    void clearCurve(std::string name);
    void removeCurve(std::string name);
    std::map<std::string, QCPGraph *>::iterator findCurve(std::string name);
    std::map<std::string, QCPGraph *>::iterator curveNameMustExist(std::string name);
    void curveNameMustNotExist(std::string name);
    QCPGraph * curveByName(std::string name);
    static QCPScatterStyle::ScatterShape scatterShape(int x);
    void addData(std::string name, const std::vector<double>& x, const std::vector<double>& y);
    void trim(QCPGraph *curve);
    void trim();
    void rescale(QCPAbstractPlottable *curve, bool onlyEnlargeX, bool onlyEnlargeY);
    void rescale(bool onlyEnlargeX, bool onlyEnlargeY);

    friend class UIFunctions;
    friend class MyCustomPlot;
};

#endif // PLOT_H_INCLUDED

