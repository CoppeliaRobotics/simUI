#ifndef UIPROXY_H_INCLUDED
#define UIPROXY_H_INCLUDED

#include <map>

#include <QObject>
#include <QString>
#include <QWidget>

#include "Proxy.h"
#include "stubs.h"
#include "widgets/all.h"

class UIProxy : public QObject
{
    Q_OBJECT

public:
    virtual ~UIProxy();

    static UIProxy * getInstance(QObject *parent = 0);
    static void destroyInstance();

private:
    UIProxy(QObject *parent = 0);

    static UIProxy *instance;

public:
    static QWidget *vrepMainWindow;
    static simFloat wheelZoomFactor;

public slots:
    void onDestroy(Proxy *proxy);

private slots:
    void onCreate(Proxy *proxy);
    void onButtonClick();
    void onValueChange(int value);
    void onValueChange(double value);
    void onValueChange(QString value);
    void onEditingFinished();
    void onPlottableClick(QCPAbstractPlottable *plottable, int index, QMouseEvent *event);
    void onLegendClick(QCPLegend *legend, QCPAbstractLegendItem *item, QMouseEvent *event);
    // ---
    void onShowWindow(Window *window);
    void onHideWindow(Window *window);
    void onSetPosition(Window *window, int x, int y);
    void onSetSize(Window *window, int w, int h);
    void onSetTitle(Window *window, std::string title);
    void onSetImage(Image *image, const char *data, int w, int h);
    void onSceneChange(Window *window, int oldSceneID, int newSceneID);
    void onSetEnabled(Widget *widget, bool enabled);
    void onSetEditValue(Edit *edit, std::string value, bool suppressSignals);
    void onSetSpinboxValue(Spinbox *spinbox, double value, bool suppressSignals);
    void onSetLabelText(Label *label, std::string text, bool suppressSignals);
    void onSetSliderValue(Slider *slider, int value, bool suppressSignals);
    void onSetCheckboxValue(Checkbox *checkbox, int value, bool suppressSignals);
    void onSetRadiobuttonValue(Radiobutton *radiobutton, int value, bool suppressSignals);
    void onInsertComboboxItem(Combobox *combobox, int index, std::string text, bool suppressSignals);
    void onRemoveComboboxItem(Combobox *combobox, int index, bool suppressSignals);
    void onSetComboboxItems(Combobox *combobox, QStringList &items, int index, bool suppressSignals);
    void onSetComboboxSelectedIndex(Combobox *combobox, int index, bool suppressSignals);
    void onSetCurrentTab(Tabs *tabs, int index, bool suppressSignals);
    void onSetWidgetVisibility(Widget *widget, bool visible);
    void onReplot(Plot *plot);
    void onAddCurve(Plot *plot, int type, std::string name, std::vector<int> color, int style, curve_options *opts);
    void onAddCurveTimePoints(Plot *plot, std::string name, std::vector<double> x, std::vector<double> y);
    void onAddCurveXYPoints(Plot *plot, std::string name, std::vector<double> t, std::vector<double> x, std::vector<double> y);
    void onClearCurve(Plot *plot, std::string name);
    void onRemoveCurve(Plot *plot, std::string name);
    void onSetPlotRanges(Plot *plot, double xmin, double xmax, double ymin, double ymax);
    void onSetPlotXRange(Plot *plot, double xmin, double xmax);
    void onSetPlotYRange(Plot *plot, double ymin, double ymax);
    void onGrowPlotRanges(Plot *plot, double xmin, double xmax, double ymin, double ymax);
    void onGrowPlotXRange(Plot *plot, double xmin, double xmax);
    void onGrowPlotYRange(Plot *plot, double ymin, double ymax);
    void onSetPlotLabels(Plot *plot, std::string x, std::string y);
    void onSetPlotXLabel(Plot *plot, std::string label);
    void onSetPlotYLabel(Plot *plot, std::string label);
    void onRescaleAxes(Plot *plot, std::string name, bool onlyEnlargeX, bool onlyEnlargeY);
    void onRescaleAxesAll(Plot *plot, bool onlyEnlargeX, bool onlyEnlargeY);
    void onSetMouseOptions(Plot *plot, bool panX, bool panY, bool zoomX, bool zoomY);
    void onSetLegendVisibility(Plot *plot, bool visible);

signals:
    void buttonClick(Widget *widget);
    void valueChange(Widget *widget, int value);
    void valueChange(Widget *widget, double value);
    void valueChange(Widget *widget, QString value);
    void editingFinished(Edit *edit, QString value);
    void windowClose(Window *window);
    void loadImageFromFile(Image *image, const char *filename, int w, int h);
    void plottableClick(Plot *plot, std::string name, int index, double x, double y);
    void legendClick(Plot *plot, std::string name);
};

#endif // UIPROXY_H_INCLUDED

