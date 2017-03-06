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

#include "QCustomPlot.h"

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

    void addCurve(std::string name, QCPGraph *curve);
    void removeCurve(std::string name);
    QCPGraph * curveByName(std::string name);

    static QCPScatterStyle::ScatterShape scatterShape(int x);
    void trim(QCPGraph *curve);

    friend class UIFunctions;
};

#endif // PLOT_H_INCLUDED

