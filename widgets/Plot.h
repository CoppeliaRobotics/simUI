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

typedef std::map<std::string, QCPAbstractPlottable*> CurveMap;

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

    CurveMap curveByName_;

public:
    Plot();
    virtual ~Plot();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    void replot(bool queue = true);

    void addCurve(int type, std::string name, std::vector<int> color, int style, curve_options *opts);
    void setCurveCommonOptions(QCPAbstractPlottable *curve, std::string name, std::vector<int> color, int style, curve_options *opts);
    QCPGraph * addTimeCurve(std::string name, std::vector<int> color, int style, curve_options *opts);
    QCPCurve * addXYCurve(std::string name, std::vector<int> color, int style, curve_options *opts);
    void clearCurve(std::string name);
    void removeCurve(std::string name);
    CurveMap::iterator findCurve(std::string name);
    CurveMap::iterator curveNameMustExist(std::string name);
    void curveNameMustNotExist(std::string name);
    QCPAbstractPlottable * curveByName(std::string name);
    static QCPScatterStyle::ScatterShape scatterShape(int x);
    void addTimeData(std::string name, const std::vector<double>& x, const std::vector<double>& y);
    void addXYData(std::string name, const std::vector<double>& t, const std::vector<double>& x, const std::vector<double>& y);
    void setXRange(double min, double max);
    void setYRange(double min, double max);
    void setXLabel(std::string label);
    void setYLabel(std::string label);
    void rescaleAxes(std::string name, bool onlyEnlargeX, bool onlyEnlargeY);
    void rescaleAxes(QCPAbstractPlottable *curve, bool onlyEnlargeX, bool onlyEnlargeY);
    void rescaleAxesAll(bool onlyEnlargeX, bool onlyEnlargeY);
    void setMouseOptions(bool panX, bool panY, bool zoomX, bool zoomY);
    void setLegendVisibility(bool visible);

    friend class UIFunctions;
    friend class MyCustomPlot;
};

#endif // PLOT_H_INCLUDED

