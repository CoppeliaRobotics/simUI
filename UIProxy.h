#ifndef UIPROXY_H_INCLUDED
#define UIPROXY_H_INCLUDED

#include <map>

#include <QObject>
#include <QString>
#include <QWidget>

#include "Proxy.h"
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

public slots:

private slots:
    void onCreate(Proxy *proxy);
    void onButtonClick();
    void onValueChange(int value);
    void onValueChange(QString value);
    void onEditingFinished();
    void onDestroy(Proxy *proxy);
    void onShowWindow(Window *window);
    void onHideWindow(Window *window);
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

signals:
    void buttonClick(Widget *widget);
    void valueChange(Widget *widget, int value);
    void valueChange(Widget *widget, QString value);
    void editingFinished(Edit *edit, QString value);
    void windowClose(Window *window);
    void loadImageFromFile(Image *image, const char *filename, int w, int h);
};

#endif // UIPROXY_H_INCLUDED

