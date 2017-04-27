#ifndef UIFUNCTIONS_H_INCLUDED
#define UIFUNCTIONS_H_INCLUDED

#include <QObject>
#include <QString>

#include "Proxy.h"
#include "stubs.h"
#include "widgets/all.h"

class UIFunctions : public QObject
{
    Q_OBJECT

public:
    virtual ~UIFunctions();

    static UIFunctions * getInstance(QObject *parent = 0);
    static void destroyInstance();

    void connectSignals();

private:
    UIFunctions(QObject *parent = 0);

    static UIFunctions *instance;

public slots:

private slots:
    void onButtonClick(Widget *widget);
    void onValueChangeInt(Widget *widget, int value);
    void onValueChangeDouble(Widget *widget, double value);
    void onValueChangeString(Widget *widget, QString value);
    void onEditingFinished(Edit *edit, QString value);
    void onWindowClose(Window *window);
    void onLoadImageFromFile(Image *image, const char *filename, int w, int h);
    void onPlottableClick(Plot *plot, std::string name, int index, double x, double y);
    void onLegendClick(Plot *plot, std::string name);
    void onCellActivate(Table *table, int row, int col, std::string text);
    void onSelectionChange(Table *table, int row, int col);
    void onMouseEvent(Image *image, int type, bool shift, bool control, int x, int y);

signals:
    void create(Proxy *proxy);
    void destroy(Proxy *proxy);
    void showWindow(Window *window);
    void hideWindow(Window *window);
    void setPosition(Window *window, int x, int y);
    void setSize(Window *window, int w, int h);
    void setTitle(Window *window, std::string title);
    void setImage(Image *image, const char *data, int w, int h);
    void sceneChange(Window *window, int oldSceneID, int newSceneID);
    void setEnabled(Widget *widget, bool enabled);
    void setEditValue(Edit *edit, std::string value, bool suppressSignals);
    void setSpinboxValue(Spinbox *spinbox, double value, bool suppressSignals);
    void setLabelText(Label *label, std::string text, bool suppressSignals);
    void setSliderValue(Slider *slider, int value, bool suppressSignals);
    void setCheckboxValue(Checkbox *checkbox, int value, bool suppressSignals);
    void setRadiobuttonValue(Radiobutton *radiobutton, int value, bool suppressSignals);
    void insertComboboxItem(Combobox *combobox, int index, std::string text, bool suppressSignals);
    void removeComboboxItem(Combobox *combobox, int index, bool suppressSignals);
    void setComboboxItems(Combobox *combobox, QStringList &items, int index, bool suppressEvents);
    void setComboboxSelectedIndex(Combobox *combobox, int index, bool suppressEvents);
    void setCurrentTab(Tabs *tabs, int index, bool suppressSignals);
    void setWidgetVisibility(Widget *widget, bool visible);
    void replot(Plot *plot);
    void addCurve(Plot *plot, int type, std::string name, std::vector<int> color, int style, curve_options *opts);
    void addCurveTimePoints(Plot *plot, std::string name, std::vector<double> x, std::vector<double> y);
    void addCurveXYPoints(Plot *plot, std::string name, std::vector<double> t, std::vector<double> x, std::vector<double> y);
    void clearCurve(Plot *plot, std::string name);
    void removeCurve(Plot *plot, std::string name);
    void setPlotRanges(Plot *plot, double xmin, double xmax, double ymin, double ymax);
    void setPlotXRange(Plot *plot, double xmin, double xmax);
    void setPlotYRange(Plot *plot, double ymin, double ymax);
    void growPlotRanges(Plot *plot, double xmin, double xmax, double ymin, double ymax);
    void growPlotXRange(Plot *plot, double xmin, double xmax);
    void growPlotYRange(Plot *plot, double ymin, double ymax);
    void setPlotLabels(Plot *plot, std::string x, std::string y);
    void setPlotXLabel(Plot *plot, std::string label);
    void setPlotYLabel(Plot *plot, std::string label);
    void rescaleAxes(Plot *plot, std::string name, bool onlyEnlargeX, bool onlyEnlargeY);
    void rescaleAxesAll(Plot *plot, bool onlyEnlargeX, bool onlyEnlargeY);
    void setMouseOptions(Plot *plot, bool panX, bool panY, bool zoomX, bool zoomY);
    void setLegendVisibility(Plot *plot, bool visible);
    void clearTable(Table *table);
    void setRowCount(Table *table, int count);
    void setColumnCount(Table *table, int count);
    void setItem(Table *table, int row, int col, std::string text);
    void setRowHeaderText(Table *table, int row, std::string text);
    void setColumnHeaderText(Table *table, int col, std::string text);
    void setItemEditable(Table *table, int row, int col, bool editable);
    void restoreState(Table *table, std::string state);
    void setRowHeight(Table *table, int row, int min_size, int max_size);
    void setColumnWidth(Table *table, int column, int min_size, int max_size);
};

#endif // UIFUNCTIONS_H_INCLUDED

