#ifndef UIPROXY_H_INCLUDED
#define UIPROXY_H_INCLUDED

#include "config.h"

#include <map>

#include <QObject>
#include <QString>
#include <QWidget>

#include "Proxy.h"
#include "stubs.h"
#include "widgets/all.h"

class UI : public QObject
{
    Q_OBJECT

public:
    virtual ~UI();

    static UI * getInstance(QObject *parent = 0);
    static void destroyInstance();

private:
    UI(QObject *parent = 0);

    static UI *instance;

public:
    static QWidget *simMainWindow;
    static double wheelZoomFactor;

public slots:
    void onMsgBox(int type, int buttons, std::string title, std::string message, int *result);
    void onFileDialog(int type, std::string title, std::string startPath, std::string initName, std::string extName, std::string ext, bool native, std::vector<std::string> *result);
    void onColorDialog(std::vector<float> initColor, std::string title, bool showAlphaChannel, bool native, std::vector<float> *result);
    void onInputDialog(std::string initValue, std::string label, std::string title, bool *ok, std::string *result);

    void onDestroy(Proxy *proxy);
    void onCreate(Proxy *proxy);

#if WIDGET_BUTTON || WIDGET_RADIOBUTTON
    void onButtonClick();
#endif

#if WIDGET_LABEL
    void onLinkActivated(const QString &link);
#endif

    void onValueChangeInt(int value);
    void onValueChangeDouble(double value);
    void onValueChangeString(QString value);

#if WIDGET_EDIT
    void onEditingFinished();
    void evaluateExpression(Edit *edit, QString text);
#endif

#if WIDGET_PLOT
    void onPlottableClick(QCPAbstractPlottable *plottable, int index, QMouseEvent *event);
    void onLegendClick(QCPLegend *legend, QCPAbstractLegendItem *item, QMouseEvent *event);
#endif

#if WIDGET_TABLE
    void onCellActivate(int row, int col);
    void onTableSelectionChange();
#endif

#if WIDGET_TREE
    void onTreeSelectionChange();
#endif

#if WIDGET_IMAGE || WIDGET_SVG
    void onMouseEvent(Widget *widget, int type, bool shift, bool control, int x, int y);
#endif

#if WIDGET_TEXTBROWSER
    void onTextChanged();
    void onAnchorClicked(const QUrl &link);
#endif

#if WIDGET_SCENE3D
    void onViewCenterChanged(const QVector3D &viewCenter);
    void onPositionChanged(const QVector3D &position);
    void onScene3DObjectClicked(Qt3DRender::QPickEvent *pick);
#endif

    // ---

    void onSetStyleSheet(Widget *widget, std::string styleSheet);

#if WIDGET_BUTTON
    void onSetButtonText(Button *button, std::string text);
    void onSetButtonPressed(Button *button, bool pressed);
#endif

    void onShowWindow(Window *window);
    void onAdjustSize(Window *window);
    void onHideWindow(Window *window);
    void onSetPosition(Window *window, int x, int y);
    void onSetSize(Window *window, int w, int h);
    void onSetTitle(Window *window, std::string title);
    void onSetWindowEnabled(Window *window, bool enabled);

#if WIDGET_IMAGE
    void onSetImage(Image *image, const char *data, int w, int h);
#endif

    void onSceneChange(Window *window, int oldSceneID, int newSceneID);
    void onSetEnabled(Widget *widget, bool enabled);

#if WIDGET_EDIT
    void onSetEditValue(Edit *edit, std::string value, bool suppressSignals);
    void onSetEvaluationResult(Edit *edit, QString txt);
#endif

#if WIDGET_SPINBOX
    void onSetSpinboxValue(Spinbox *spinbox, double value, bool suppressSignals);
#endif

#if WIDGET_LABEL
    void onSetLabelText(Label *label, std::string text, bool suppressSignals);
#endif

#if WIDGET_HSLIDER || WIDGET_VSLIDER
    void onSetSliderValue(Slider *slider, int value, bool suppressSignals);
#endif

#if WIDGET_CHECKBOX
    void onSetCheckboxValue(Checkbox *checkbox, Qt::CheckState value, bool suppressSignals);
#endif

#if WIDGET_RADIOBUTTON
    void onSetRadiobuttonValue(Radiobutton *radiobutton, bool value, bool suppressSignals);
#endif

#if WIDGET_COMBOBOX
    void onInsertComboboxItem(Combobox *combobox, int index, std::string text, bool suppressSignals);
    void onRemoveComboboxItem(Combobox *combobox, int index, bool suppressSignals);
    void onSetComboboxItems(Combobox *combobox, std::vector<std::string> items, int index, bool suppressSignals);
    void onSetComboboxSelectedIndex(Combobox *combobox, int index, bool suppressSignals);
#endif

#if WIDGET_TABS
    void onSetCurrentTab(Tabs *tabs, int index, bool suppressSignals);
#endif

    void onSetWidgetVisibility(Widget *widget, bool visible);

#if WIDGET_PLOT
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
#endif

#if WIDGET_TABLE
    void onClearTable(Table *table, bool suppressSignals);
    void onSetRowCount(Table *table, int count, bool suppressSignals);
    void onSetColumnCountTable(Table *table, int count, bool suppressSignals);
    void onSetItem(Table *table, int row, int column, std::string text, bool suppressSignals);
    void onSetItemImage(Table *table, int row, int column, std::string data, int width, int height, bool suppressSignals);
    void onSetItems(Table *table, std::string data, bool suppressSignals);
    void onSetRowHeaderText(Table *table, int row, std::string text);
    void onSetColumnHeaderTextTable(Table *table, int column, std::string text);
    void onSetItemEditable(Table *table, int row, int column, bool editable);
    void onRestoreStateTable(Table *table, std::string state);
    void onSetRowHeight(Table *table, int row, int min_size, int max_size);
    void onSetColumnWidthTable(Table *table, int column, int min_size, int max_size);
    void onSetTableSelection(Table *table, int row, int column, bool suppressSignals);
#endif

#if WIDGET_PROGRESSBAR
    void onSetProgress(Progressbar *progressbar, int value);
#endif

#if WIDGET_PROPERTIES
    void onSetProperties(Properties *properties, std::vector<std::string> pnames, std::vector<std::string> ptypes, std::vector<std::string> pvalues, std::vector<int> pflags, std::vector<std::string> pdisplayk, std::vector<std::string> pdisplayv, bool suppressSignals);
    void onSetPropertiesRow(Properties *properties, int row, std::string pname, std::string ptype, std::string pvalue, int pflags, std::string pdisplayk, std::string pdisplayv, bool suppressSignals);
    void onSetPropertiesSelection(Properties *properties, int row, bool suppressSignals);
    void onSetPropertiesContextMenu(Properties *properties, std::vector<std::string> keys, std::vector<std::string> titles);
    void onSetPropertiesState(Properties *properties, std::string state);
    void onGetPropertiesState(Properties *properties, std::string *state);
    void onPropertiesSelectionChange(const QItemSelection &selected, const QItemSelection &deselected);
    void onPropertiesDoubleClick(const QModelIndex &index);
#endif

#if WIDGET_TREE
    void onSetColumnCountTree(Tree *tree, int count, bool suppressSignals);
    void onSetColumnHeaderTextTree(Tree *tree, int column, std::string text);
    void onRestoreStateTree(Tree *tree, std::string state);
    void onSetColumnWidthTree(Tree *tree, int column, int min_size, int max_size);
    void onClearTree(Tree *tree, bool suppressSignals);
    void onAddTreeItem(Tree *tree, int item_id, int parent_id, std::vector<std::string> text, bool expanded, bool suppressSignals);
    void onUpdateTreeItemText(Tree *tree, int item_id, std::vector<std::string> text);
    void onUpdateTreeItemParent(Tree *tree, int item_id, int parent_id, bool suppressSignals);
    void onRemoveTreeItem(Tree *tree, int item_id, bool suppressSignals);
    void onSetTreeSelection(Tree *tree, int item_id, bool suppressSignals);
    void onExpandAll(Tree *tree, bool suppressSignals);
    void onCollapseAll(Tree *tree, bool suppressSignals);
    void onExpandToDepth(Tree *tree, int depth, bool suppressSignals);
#endif

#if WIDGET_TEXTBROWSER
    void onSetText(TextBrowser *textbrowser, std::string text, bool suppressSignals);
    void onAppendText(TextBrowser *textbrowser, std::string text, bool suppressSignals);
    void onSetUrl(TextBrowser *textbrowser, std::string url);
#endif

#if WIDGET_SCENE3D
    void onAddScene3DNode(Scene3D *scene3d, int id, int parentId, int type);
    void onRemoveScene3DNode(Scene3D *scene3d, int id);
    void onSetScene3DNodeEnabled(Scene3D *scene3d, int id, bool enabled);
    void onSetScene3DIntParam(Scene3D *scene3d, int id, std::string param, int value);
    void onSetScene3DFloatParam(Scene3D *scene3d, int id, std::string param, double value);
    void onSetScene3DStringParam(Scene3D *scene3d, int id, std::string param, std::string value);
    void onSetScene3DVector2Param(Scene3D *scene3d, int id, std::string param, double x, double y);
    void onSetScene3DVector3Param(Scene3D *scene3d, int id, std::string param, double x, double y, double z);
    void onSetScene3DVector4Param(Scene3D *scene3d, int id, std::string param, double x, double y, double z, double w);
#endif

#if WIDGET_SVG
    void onSvgLoadFile(SVG *svg, const QString &file);
    void onSvgLoadData(SVG *svg, const QByteArray &data);
#endif

    void onSetClipboardText(QString text);

signals:
    void buttonClick(Widget *widget);
    void linkActivated(Widget *widget, QString link);
    void valueChangeInt(Widget *widget, int value);
    void valueChangeDouble(Widget *widget, double value);
    void valueChangeString(Widget *widget, QString value);

#if WIDGET_EDIT
    void editingFinished(Edit *edit, QString value);
    void evaluateExpressionInSandbox(Edit *edit, QString text);
#endif

    void windowClose(Window *window);

#if WIDGET_IMAGE
    void loadImageFromFile(Image *image, const char *filename, int w, int h);
#endif

#if WIDGET_IMAGE || WIDGET_SVG
    void mouseEvent(Widget *widget, int type, bool shift, bool control, int x, int y);
#endif

#if WIDGET_PLOT
    void plottableClick(Plot *plot, std::string name, int index, double x, double y);
    void legendClick(Plot *plot, std::string name);
#endif

#if WIDGET_TABLE
    void cellActivate(Table *table, int row, int col, std::string value);
    void tableSelectionChange(Table *table, int row, int col);
#endif

#if WIDGET_PROPERTIES
    void propertiesSelectionChange(Properties *properties, int row);
    void propertiesDoubleClick(Properties *properties, int row, int col);
    void propertiesContextMenuTriggered(Properties *properties, std::string key);
    void propertiesPropertyEdit(Properties *properties, std::string key, std::string value);
#endif

#if WIDGET_TREE
    void treeSelectionChange(Tree *tree, int id);
#endif

    void keyPressed(Widget *widget, int key, std::string text);

#if WIDGET_SCENE3D
    void scene3DObjectClick(Scene3D *scene3d, int id);
#endif
};

#endif // UIPROXY_H_INCLUDED

