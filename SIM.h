#ifndef UIFUNCTIONS_H_INCLUDED
#define UIFUNCTIONS_H_INCLUDED

#include "config.h"

#include <QObject>
#include <QString>

#include "Proxy.h"
#include "stubs.h"
#include "widgets/all.h"

class SIM : public QObject
{
    Q_OBJECT

public:
    virtual ~SIM();

    static SIM * getInstance(QObject *parent = 0);
    static void destroyInstance();

    void connectSignals();

private:
    SIM(QObject *parent = 0);

    static SIM *instance;

public slots:

private slots:
#if WIDGET_BUTTON
    void onButtonClick(Widget *widget);
#endif

#if WIDGET_LABEL
    void onLinkActivated(Widget *widget, QString link);
#endif

    void onValueChangeInt(Widget *widget, int value);
    void onValueChangeDouble(Widget *widget, double value);
    void onValueChangeString(Widget *widget, QString value);

#if WIDGET_EDIT
    void onEditingFinished(Edit *edit, QString value);
#endif

    void onWindowClose(Window *window);

#if WIDGET_IMAGE
    void onLoadImageFromFile(Image *image, const char *filename, int w, int h, bool resize);
#endif

#if WIDGET_PLOT
    void onPlottableClick(Plot *plot, std::string name, int index, double x, double y);
    void onLegendClick(Plot *plot, std::string name);
#endif

#if WIDGET_TABLE
    void onCellActivate(Table *table, int row, int col, std::string text);
    void onSelectionChangeTable(Table *table, int row, int col);
#endif

#if WIDGET_TREE
    void onSelectionChangeTree(Tree *tree, int id);
#endif

#if WIDGET_IMAGE
    void onMouseEvent(Widget *widget, int type, bool shift, bool control, int x, int y);
#endif

    void onKeyPress(Widget *widget, int key, std::string text);

#if WIDGET_SCENE3D
    void onScene3DObjectClick(Scene3D *scene3d, int id);
#endif

signals:
    void msgBox(int type, int buttons, std::string title, std::string message, int *result);
    void fileDialog(int type, std::string title, std::string startPath, std::string initName, std::string extName, std::string ext, bool native, std::vector<std::string> *result);
    void colorDialog(std::vector<float> initColor, std::string title, bool showAlphaChannel, bool native, std::vector<float> *result);
    void inputDialog(std::string initValue, std::string label, std::string title, bool *ok, std::string *result);
    void create(Proxy *proxy);
    void destroy(Proxy *proxy);
    void setStyleSheet(Widget *widget, std::string styleSheet);
    void showWindow(Window *window);
    void adjustSize(Window *window);
    void hideWindow(Window *window);
    void setPosition(Window *window, int x, int y);
    void setSize(Window *window, int w, int h);
    void setTitle(Window *window, std::string title);
    void setWindowEnabled(Window *window, bool enabled);

#if WIDGET_IMAGE
    void setImage(Image *image, const char *data, int w, int h, bool resize);
#endif

    void sceneChange(Window *window, int oldSceneID, int newSceneID);
    void setEnabled(Widget *widget, bool enabled);

#if WIDGET_BUTTON
    void setButtonText(Button *button, std::string text);
    void setButtonPressed(Button *button, bool pressed);
#endif

#if WIDGET_EDIT
    void setEditValue(Edit *edit, std::string value, bool suppressSignals);
#endif

#if WIDGET_SPINBOX
    void setSpinboxValue(Spinbox *spinbox, double value, bool suppressSignals);
#endif

#if WIDGET_LABEL
    void setLabelText(Label *label, std::string text, bool suppressSignals);
#endif

#if WIDGET_HSLIDER || WIDGET_VSLIDER
    void setSliderValue(Slider *slider, int value, bool suppressSignals);
#endif

#if WIDGET_CHECKBOX
    void setCheckboxValue(Checkbox *checkbox, Qt::CheckState value, bool suppressSignals);
#endif

#if WIDGET_RADIOBUTTON
    void setRadiobuttonValue(Radiobutton *radiobutton, bool value, bool suppressSignals);
#endif

#if WIDGET_COMBOBOX
    void insertComboboxItem(Combobox *combobox, int index, std::string text, bool suppressSignals);
    void removeComboboxItem(Combobox *combobox, int index, bool suppressSignals);
    void setComboboxItems(Combobox *combobox, std::vector<std::string> items, int index, bool suppressEvents);
    void setComboboxSelectedIndex(Combobox *combobox, int index, bool suppressEvents);
#endif

#if WIDGET_TABS
    void setCurrentTab(Tabs *tabs, int index, bool suppressSignals);
#endif

    void setWidgetVisibility(Widget *widget, bool visible);

#if WIDGET_PLOT
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
#endif

#if WIDGET_TABLE
    void clearTable(Table *table, bool suppressSignals);
    void setRowCount(Table *table, int count, bool suppressSignals);
    void setColumnCountTable(Table *table, int count, bool suppressSignals);
    void setItem(Table *table, int row, int col, std::string text, bool suppressSignals);
    void setItemImage(Table *table, int row, int col, std::string data, int width, int height, bool suppressSignals);
    void setRowHeaderText(Table *table, int row, std::string text);
    void setColumnHeaderTextTable(Table *table, int col, std::string text);
    void setItemEditable(Table *table, int row, int col, bool editable);
    void restoreStateTable(Table *table, std::string state);
    void setRowHeight(Table *table, int row, int min_size, int max_size);
    void setColumnWidthTable(Table *table, int column, int min_size, int max_size);
    void setTableSelection(Table *table, int row, int col, bool suppressSignals);
#endif

#if WIDGET_PROGRESSBAR
    void setProgress(Progressbar *progressbar, int value);
#endif

#if WIDGET_TREE
    void setColumnCountTree(Tree *tree, int count, bool suppressSignals);
    void setColumnHeaderTextTree(Tree *tree, int col, std::string text);
    void restoreStateTree(Tree *tree, std::string state);
    void setColumnWidthTree(Tree *tree, int column, int min_size, int max_size);
    void clearTree(Tree *tree, bool suppressSignals);
    void addTreeItem(Tree *tree, int item_id, int parent_id, std::vector<std::string> text, bool expanded, bool suppressEvents);
    void updateTreeItemText(Tree *tree, int item_id, std::vector<std::string> text);
    void updateTreeItemParent(Tree *tree, int item_id, int parent_id, bool suppressSignals);
    void removeTreeItem(Tree *tree, int item_id, bool suppressEvents);
    void setTreeSelection(Tree *tree, int item_id, bool suppressSignals);
    void expandAll(Tree *tree, bool suppressSignals);
    void collapseAll(Tree *tree, bool suppressSignals);
    void expandToDepth(Tree *tree, int depth, bool suppressSignals);
#endif

#if WIDGET_TEXTBROWSER
    void setText(TextBrowser *textbrowser, std::string text, bool suppressSignals);
    void setUrl(TextBrowser *textbrowser, std::string url);
#endif

#if WIDGET_SCENE3D
    void addScene3DNode(Scene3D *scene3d, int id, int parentId, int type);
    void removeScene3DNode(Scene3D *scene3d, int id);
    void setScene3DNodeEnabled(Scene3D *scene3d, int id, bool enabled);
    void setScene3DIntParam(Scene3D *scene3d, int id, std::string param, int value);
    void setScene3DFloatParam(Scene3D *scene3d, int id, std::string param, double value);
    void setScene3DStringParam(Scene3D *scene3d, int id, std::string param, std::string value);
    void setScene3DVector2Param(Scene3D *scene3d, int id, std::string param, double x, double y);
    void setScene3DVector3Param(Scene3D *scene3d, int id, std::string param, double x, double y, double z);
    void setScene3DVector4Param(Scene3D *scene3d, int id, std::string param, double x, double y, double z, double w);
#endif

#if WIDGET_SVG
    void svgLoadFile(SVG *svg, const QString &file);
    void svgLoadData(SVG *svg, const QByteArray &data);
#endif
};

#endif // UIFUNCTIONS_H_INCLUDED

