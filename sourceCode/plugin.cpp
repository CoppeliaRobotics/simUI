#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include <QVector>
#include <QThread>
#include <QSlider>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QWidget>
#include <QLabel>
#include <QGroupBox>
#include <QComboBox>
#include <QDialog>
#include <QImageReader>
#include <QLibraryInfo>
#include <QString>
#include <QByteArray>
#include <QCryptographicHash>

#include "tinyxml2.h"

#include <simPlusPlus/Plugin.h>
#include <simPlusPlus/Handles.h>
#include "plugin.h"
#include "config.h"
#include "stubs.h"
#include "Proxy.h"
#include "SIM.h"
#include "UI.h"
#include "widgets/all.h"

#ifdef ENABLE_SIGNAL_SPY
#include "signal_spy.h"
#endif

using namespace simExtCustomUI;

class Plugin : public sim::Plugin
{
public:
    void onInit()
    {
        if(sim::getBoolParam(sim_boolparam_headless))
            throw std::runtime_error("doesn't work in headless mode");

        if(!registerScriptStuff())
            throw std::runtime_error("failed to register script stuff");

        setExtVersion("Custom User-Interface Plugin");
        setBuildDate(BUILD_DATE);

        SIM::getInstance();

        UI::wheelZoomFactor = sim::getFloatParam(sim_floatparam_mouse_wheel_zoom_factor);

#if defined(ENABLE_SIGNAL_SPY) && !defined(NDEBUG)
        SignalSpy::start();
#endif
    }

    void onCleanup()
    {
        SIM::destroyInstance();
        SIM_THREAD = NULL;
    }

    void onUIInit()
    {
        UI::getInstance();
    }

    void onUICleanup()
    {
        // need to destroy any hanging windows here as doing that later from onCleanup() would be too late:
        for(auto proxy : handles.all())
            UI::getInstance()->onDestroy(handles.remove(proxy));

        UI::destroyInstance();
        UI_THREAD = NULL;
    }

    void onInstanceSwitch(int sceneID, int oldSceneID)
    {
        for(auto proxy : handles.all())
        {
            auto window = proxy->getWidget();
            if(window)
            {
                SIM::getInstance()->sceneChange(window, oldSceneID, sceneID);
            }
        }
    }

    void onScriptStateAboutToBeDestroyed(int scriptHandle, long long scriptUid)
    {
        for(auto proxy : handles.find(scriptHandle))
            SIM::getInstance()->destroy(handles.remove(proxy));
    }

    void msgBox(msgBox_in *in, msgBox_out *out)
    {
        sim::addLog(sim_verbosity_debug, "[enter]");
        int result;
        // this function is called also from the C API: always run it in the correct thread
        if(QThread::currentThreadId() == UI_THREAD)
            UI::getInstance()->onMsgBox(in->type, in->buttons, in->title, in->message, &result);
        else
            SIM::getInstance()->msgBox(in->type, in->buttons, in->title, in->message, &result);
        out->result = result;
        sim::addLog(sim_verbosity_debug, "[leave]");
    }

    void fileDialog(fileDialog_in *in, fileDialog_out *out)
    {
        sim::addLog(sim_verbosity_debug, "[enter]");
        std::vector<std::string> result;
        // this function is called also from the C API: always run it in the correct thread
        if(QThread::currentThreadId() == UI_THREAD)
            UI::getInstance()->onFileDialog(in->type, in->title, in->startPath, in->initName, in->extName, in->ext, in->native, &result);
        else
            SIM::getInstance()->fileDialog(in->type, in->title, in->startPath, in->initName, in->extName, in->ext, in->native, &result);
        for(auto x : result) out->result.push_back(x);
        sim::addLog(sim_verbosity_debug, "[leave]");
    }

    void colorDialog(colorDialog_in *in, colorDialog_out *out)
    {
        sim::addLog(sim_verbosity_debug, "[enter]");
        // this function is called also from the C API: always run it in the correct thread
        std::vector<float> r;
        if(QThread::currentThreadId() == UI_THREAD)
            UI::getInstance()->onColorDialog(in->initColor, in->title, in->showAlphaChannel, in->native, &r);
        else
            SIM::getInstance()->colorDialog(in->initColor, in->title, in->showAlphaChannel, in->native, &r);
        if(!r.empty()) out->result = r;
        sim::addLog(sim_verbosity_debug, "[leave]");
    }

    void inputDialog(inputDialog_in *in, inputDialog_out *out)
    {
        sim::addLog(sim_verbosity_debug, "[enter]");
        // this function is called also from the C API: always run it in the correct thread
        bool ok;
        std::string r;
        if(QThread::currentThreadId() == UI_THREAD)
            UI::getInstance()->onInputDialog(in->initValue, in->label, in->title, &ok, &r);
        else
            SIM::getInstance()->inputDialog(in->initValue, in->label, in->title, &ok, &r);
        if(ok) out->result = r;
        sim::addLog(sim_verbosity_debug, "[leave]");
    }

    void create(create_in *in, create_out *out)
    {
        ASSERT_THREAD(!UI);
        sim::addLog(sim_verbosity_debug, "[enter]");
        tinyxml2::XMLDocument xmldoc;
        tinyxml2::XMLError error = xmldoc.Parse(in->xml.c_str(), in->xml.size());

        if(error != tinyxml2::XML_NO_ERROR)
            throw std::runtime_error("XML parse error");

        tinyxml2::XMLElement *rootElement = xmldoc.FirstChildElement();
        std::map<int, Widget*> widgets;
        Window *window = new Window;
        try
        {
            window->parse(widgets, rootElement);
        }
        catch(std::exception& ex)
        {
            delete window;
            throw;
        }

        // determine wether the Proxy object should be destroyed at simulation end
        int scriptType = sim::getScriptInt32Param(in->_.scriptID, sim_scriptintparam_type);
        int sceneID = sim::getInt32Param(sim_intparam_scene_unique_id);
        sim::addLog(sim_verbosity_debug, "Creating a new Proxy object...");
        Proxy *proxy = new Proxy(sceneID, in->_.scriptID, scriptType, window, widgets);
        proxy->handle = handles.add(proxy, in->_.scriptID);
        out->uiHandle = proxy->handle;
        sim::addLog(sim_verbosity_debug, "Proxy %s created in scene %d", out->uiHandle, sceneID);

        sim::addLog(sim_verbosity_debug, "call SIM::create() (will emit the create(Proxy*) signal)...");
        SIM::getInstance()->create(proxy); // connected to UI, which
                                // will run code for creating Qt widgets in the UI thread
        sim::addLog(sim_verbosity_debug, "[leave]");
    }

    void destroy(destroy_in *in, destroy_out *out)
    {
        ASSERT_THREAD(!UI);
        sim::addLog(sim_verbosity_debug, "[enter]");

        Proxy *proxy = handles.get(in->handle);

        sim::addLog(sim_verbosity_debug, "call SIM::destroy() (will emit the destroy(Proxy*) signal)...");
        SIM::getInstance()->destroy(handles.remove(proxy)); // will also delete proxy

        sim::addLog(sim_verbosity_debug, "[leave]");
    }

    Widget* getWidget(const std::string &handle, int id)
    {
        Widget *widget = Widget::byId(handle, id, handles);
        if(!widget)
        {
            std::stringstream ss;
            ss << "invalid widget id: " << id;
            throw std::runtime_error(ss.str());
        }
        return widget;
    }

    template<typename T>
    T* getWidget(const std::string &handle, int id, const char *widget_type_name)
    {
        T *twidget = dynamic_cast<T*>(getWidget(handle, id));
        if(!twidget)
        {
            std::stringstream ss;
            ss << "invalid widget type. expected " << widget_type_name << " (in function getWidget())";
            throw std::runtime_error(ss.str());
        }
        return twidget;
    }

    template<typename T>
    T* getQWidget(const std::string &handle, int id, const char *widget_type_name)
    {
        Widget *widget = getWidget<Widget>(handle, id, widget_type_name);

        T *qwidget = dynamic_cast<T*>(widget->getQWidget());
        if(!qwidget)
        {
            std::stringstream ss;
            ss << "invalid widget type. expected " << widget_type_name << " (in function getQWidget())";
            throw std::runtime_error(ss.str());
        }

        return qwidget;
    }

    void setStyleSheet(setStyleSheet_in *in, setStyleSheet_out *out)
    {
        ASSERT_THREAD(!UI);
        Widget *widget = getWidget(in->handle, in->id);
        SIM::getInstance()->setStyleSheet(widget, in->styleSheet);
    }

    void setButtonText(setButtonText_in *in, setButtonText_out *out)
    {
#if WIDGET_BUTTON
        ASSERT_THREAD(!UI);
        Button *button = getWidget<Button>(in->handle, in->id, "button");
        SIM::getInstance()->setButtonText(button, in->text);
#endif
    }

    void setButtonPressed(setButtonPressed_in *in, setButtonPressed_out *out)
    {
#if WIDGET_BUTTON
        ASSERT_THREAD(!UI);
        Button *button = getWidget<Button>(in->handle, in->id, "button");
        SIM::getInstance()->setButtonPressed(button, in->pressed);
#endif
    }

    void getSliderValue(getSliderValue_in *in, getSliderValue_out *out)
    {
#if WIDGET_HSLIDER || WIDGET_VSLIDER
        Slider *slider = getWidget<Slider>(in->handle, in->id, "slider");
        out->value = slider->getValue();
#endif
    }

    void setSliderValue(setSliderValue_in *in, setSliderValue_out *out)
    {
#if WIDGET_HSLIDER || WIDGET_VSLIDER
        ASSERT_THREAD(!UI);
        Slider *slider = getWidget<Slider>(in->handle, in->id, "slider");
        SIM::getInstance()->setSliderValue(slider, in->value, in->suppressEvents);
#endif
    }

    void getEditValue(getEditValue_in *in, getEditValue_out *out)
    {
#if WIDGET_EDIT
        Edit *edit = getWidget<Edit>(in->handle, in->id, "edit");
        out->value = edit->getValue();
#endif
    }

    void setEditValue(setEditValue_in *in, setEditValue_out *out)
    {
#if WIDGET_EDIT
        ASSERT_THREAD(!UI);
        Edit *edit = getWidget<Edit>(in->handle, in->id, "edit");
        SIM::getInstance()->setEditValue(edit, in->value, in->suppressEvents);
#endif
    }

    void getSpinboxValue(getSpinboxValue_in *in, getSpinboxValue_out *out)
    {
#if WIDGET_SPINBOX
        Spinbox *spinbox = getWidget<Spinbox>(in->handle, in->id, "spinbox");
        out->value = spinbox->getValue();
#endif
    }

    void setSpinboxValue(setSpinboxValue_in *in, setSpinboxValue_out *out)
    {
#if WIDGET_SPINBOX
        ASSERT_THREAD(!UI);
        Spinbox *spinbox = getWidget<Spinbox>(in->handle, in->id, "spinbox");
        SIM::getInstance()->setSpinboxValue(spinbox, in->value, in->suppressEvents);
#endif
    }

    void getCheckboxValue(getCheckboxValue_in *in, getCheckboxValue_out *out)
    {
#if WIDGET_CHECKBOX
        Checkbox *checkbox = getWidget<Checkbox>(in->handle, in->id, "checkbox");
        out->value = checkbox->convertValueToInt(checkbox->getValue());
#endif
    }

    void setCheckboxValue(setCheckboxValue_in *in, setCheckboxValue_out *out)
    {
#if WIDGET_CHECKBOX
        ASSERT_THREAD(!UI);
        Checkbox *checkbox = getWidget<Checkbox>(in->handle, in->id, "checkbox");
        Qt::CheckState value = checkbox->convertValueFromInt(in->value);
        SIM::getInstance()->setCheckboxValue(checkbox, value, in->suppressEvents);
#endif
    }

    void getRadiobuttonValue(getRadiobuttonValue_in *in, getRadiobuttonValue_out *out)
    {
#if WIDGET_RADIOBUTTON
        Radiobutton *radiobutton = getWidget<Radiobutton>(in->handle, in->id, "radiobutton");
        out->value = radiobutton->convertValueToInt(radiobutton->getValue());
#endif
    }

    void setRadiobuttonValue(setRadiobuttonValue_in *in, setRadiobuttonValue_out *out)
    {
#if WIDGET_RADIOBUTTON
        ASSERT_THREAD(!UI);
        Radiobutton *radiobutton = getWidget<Radiobutton>(in->handle, in->id, "radiobutton");
        bool value = radiobutton->convertValueFromInt(in->value);
        SIM::getInstance()->setRadiobuttonValue(radiobutton, value, in->suppressEvents);
#endif
    }

    void getLabelText(getLabelText_in *in, getLabelText_out *out)
    {
#if WIDGET_LABEL
        Label *label = getWidget<Label>(in->handle, in->id, "label");
        out->text = label->getText();
#endif
    }

    void setLabelText(setLabelText_in *in, setLabelText_out *out)
    {
#if WIDGET_LABEL
        ASSERT_THREAD(!UI);
        Label *label = getWidget<Label>(in->handle, in->id, "label");
        SIM::getInstance()->setLabelText(label, in->text, in->suppressEvents);
#endif
    }

    void insertComboboxItem(insertComboboxItem_in *in, insertComboboxItem_out *out)
    {
#if WIDGET_COMBOBOX
        ASSERT_THREAD(!UI);
        Combobox *combobox = getWidget<Combobox>(in->handle, in->id, "combobox");
        SIM::getInstance()->insertComboboxItem(combobox, in->index, in->text, in->suppressEvents);
#endif
    }

    void removeComboboxItem(removeComboboxItem_in *in, removeComboboxItem_out *out)
    {
#if WIDGET_COMBOBOX
        ASSERT_THREAD(!UI);
        Combobox *combobox = getWidget<Combobox>(in->handle, in->id, "combobox");
        SIM::getInstance()->removeComboboxItem(combobox, in->index, in->suppressEvents);
#endif
    }

    void getComboboxItemCount(getComboboxItemCount_in *in, getComboboxItemCount_out *out)
    {
#if WIDGET_COMBOBOX
        Combobox *combobox = getWidget<Combobox>(in->handle, in->id, "combobox");
        out->count = combobox->count();
#endif
    }

    void getComboboxItemText(getComboboxItemText_in *in, getComboboxItemText_out *out)
    {
#if WIDGET_COMBOBOX
        Combobox *combobox = getWidget<Combobox>(in->handle, in->id, "combobox");
        out->text = combobox->itemText(in->index);
#endif
    }

    void getComboboxItems(getComboboxItems_in *in, getComboboxItems_out *out)
    {
#if WIDGET_COMBOBOX
        Combobox *combobox = getWidget<Combobox>(in->handle, in->id, "combobox");
        out->items = combobox->getItems();
#endif
    }

    void setComboboxItems(setComboboxItems_in *in, setComboboxItems_out *out)
    {
#if WIDGET_COMBOBOX
        ASSERT_THREAD(!UI);
        Combobox *combobox = getWidget<Combobox>(in->handle, in->id, "combobox");
        SIM::getInstance()->setComboboxItems(combobox, in->items, in->index, in->suppressEvents);
#endif
    }

    void setComboboxSelectedIndex(setComboboxSelectedIndex_in *in, setComboboxSelectedIndex_out *out)
    {
#if WIDGET_COMBOBOX
        ASSERT_THREAD(!UI);
        Combobox *combobox = getWidget<Combobox>(in->handle, in->id, "combobox");
        SIM::getInstance()->setComboboxSelectedIndex(combobox, in->index, in->suppressEvents);
#endif
    }

    void getComboboxSelectedIndex(getComboboxSelectedIndex_in *in, getComboboxSelectedIndex_out *out)
    {
#if WIDGET_COMBOBOX
        ASSERT_THREAD(!UI);
        Combobox *combobox = getWidget<Combobox>(in->handle, in->id, "combobox");
        out->index = combobox->getSelectedIndex();
#endif
    }

    void adjustSize(adjustSize_in *in, adjustSize_out *out)
    {
        ASSERT_THREAD(!UI);
        Proxy *proxy = handles.get(in->handle);
        SIM::getInstance()->adjustSize(proxy->getWidget());
    }

    void hide(hide_in *in, hide_out *out)
    {
        ASSERT_THREAD(!UI);
        Proxy *proxy = handles.get(in->handle);
        SIM::getInstance()->hideWindow(proxy->getWidget());
    }

    void show(show_in *in, show_out *out)
    {
        ASSERT_THREAD(!UI);
        Proxy *proxy = handles.get(in->handle);
        SIM::getInstance()->showWindow(proxy->getWidget());
    }

    void isVisible(isVisible_in *in, isVisible_out *out)
    {
        Proxy *proxy = handles.get(in->handle);
        out->visibility = proxy->getWidget()->getQWidget()->isVisible();
    }

    void getPosition(getPosition_in *in, getPosition_out *out)
    {
        Proxy *proxy = handles.get(in->handle);
        Window *window = proxy->getWidget();
        out->x = window->pos().x();
        out->y = window->pos().y();
    }

    void setPosition(setPosition_in *in, setPosition_out *out)
    {
        ASSERT_THREAD(!UI);
        Proxy *proxy = handles.get(in->handle);
        Window *window = proxy->getWidget();
        SIM::getInstance()->setPosition(window, in->x, in->y);
    }

    void getSize(getSize_in *in, getSize_out *out)
    {
        Proxy *proxy = handles.get(in->handle);
        Window *window = proxy->getWidget();
        out->w = window->size().width();
        out->h = window->size().height();
    }

    void setSize(setSize_in *in, setSize_out *out)
    {
        ASSERT_THREAD(!UI);
        Proxy *proxy = handles.get(in->handle);
        Window *window = proxy->getWidget();
        SIM::getInstance()->setSize(window, in->w, in->h);
    }

    void getTitle(getTitle_in *in, getTitle_out *out)
    {
        Proxy *proxy = handles.get(in->handle);
        QWidget *window = proxy->getWidget()->getQWidget();
        out->title = static_cast<QDialog*>(window)->windowTitle().toStdString();
    }

    void setTitle(setTitle_in *in, setTitle_out *out)
    {
        ASSERT_THREAD(!UI);
        Proxy *proxy = handles.get(in->handle);
        Window *window = proxy->getWidget();
        SIM::getInstance()->setTitle(window, in->title);
    }

    void setWindowEnabled(setWindowEnabled_in *in, setWindowEnabled_out *out)
    {
        ASSERT_THREAD(!UI);
        Proxy *proxy = handles.get(in->handle);
        Window *window = proxy->getWidget();
        SIM::getInstance()->setWindowEnabled(window, in->enabled);
    }

    void setImageData(setImageData_in *in, setImageData_out *out)
    {
#if WIDGET_IMAGE
        ASSERT_THREAD(!UI);
        Image *imageWidget = dynamic_cast<Image*>(Widget::byId(in->handle, in->id, handles));
        if(!imageWidget)
        {
            std::stringstream ss;
            ss << "invalid image widget id: " << in->id;
            throw std::runtime_error(ss.str());
        }

        int bpp = 3; // bytes per pixel
        int sz = in->width * in->height * bpp;
        if(in->data.size() != sz)
        {
            std::stringstream ss;
            ss << "bad image size. expected: " << sz << ". got: " << in->data.size() << ".";
            throw std::runtime_error(ss.str());
        }

        char *img = (char*)sim::createBuffer(sz);
        std::memcpy(img, in->data.c_str(), sz);
        sim::transformImage((unsigned char *)img, {in->width, in->height}, 4);

        SIM::getInstance()->setImage(imageWidget, img, in->width, in->height);
#endif
    }

    void setEnabled(setEnabled_in *in, setEnabled_out *out)
    {
        ASSERT_THREAD(!UI);
        Widget *widget = Widget::byId(in->handle, in->id, handles);
        if(!widget)
        {
            std::stringstream ss;
            ss << "invalid widget id: " << in->id;
            throw std::runtime_error(ss.str());
        }

        SIM::getInstance()->setEnabled(widget, in->enabled);
    }

    void getCurrentTab(getCurrentTab_in *in, getCurrentTab_out *out)
    {
#if WIDGET_TABS
        QTabWidget *tabs = getQWidget<QTabWidget>(in->handle, in->id, "tabs");
        out->index = tabs->currentIndex();
#endif
    }

    void setCurrentTab(setCurrentTab_in *in, setCurrentTab_out *out)
    {
#if WIDGET_TABS
        ASSERT_THREAD(!UI);
        Tabs *tabs = getWidget<Tabs>(in->handle, in->id, "tabs");
        SIM::getInstance()->setCurrentTab(tabs, in->index, in->suppressEvents);
#endif
    }

    void getWidgetVisibility(getWidgetVisibility_in *in, getWidgetVisibility_out *out)
    {
        QWidget *widget = getQWidget<QWidget>(in->handle, in->id, "widget");
        out->visibility = widget->isVisible();
    }

    void setWidgetVisibility(setWidgetVisibility_in *in, setWidgetVisibility_out *out)
    {
        ASSERT_THREAD(!UI);
        Widget *widget = Widget::byId(in->handle, in->id, handles);
        if(!widget)
        {
            std::stringstream ss;
            ss << "invalid widget id: " << in->id;
            throw std::runtime_error(ss.str());
        }

        SIM::getInstance()->setWidgetVisibility(widget, in->visibility);
    }

    void getCurrentEditWidget(getCurrentEditWidget_in *in, getCurrentEditWidget_out *out)
    {
#if WIDGET_EDIT
        Proxy *proxy = handles.get(in->handle);
        QWidget *window = proxy->getWidget()->getQWidget();
        QLineEdit* qedit = NULL;
        QList<QLineEdit*> wl = window->findChildren<QLineEdit*>(QString());
        for(int i = 0; i < wl.size(); i++)
        {
            if(wl[i]->selectedText().size() > 0)
            {
                qedit = wl[i];
                break;
            }
        }
        Widget *edit = Widget::byQWidget(qedit);
        if(edit)
            out->id = edit->getId();
#endif
    }

    void setCurrentEditWidget(setCurrentEditWidget_in *in, setCurrentEditWidget_out *out)
    {
#if WIDGET_EDIT
        if(in->id == -1) return;
        Edit *edit = getWidget<Edit>(in->handle, in->id, "edit");
        QLineEdit *qedit = static_cast<QLineEdit*>(edit->getQWidget());
        qedit->setFocus();
        qedit->selectAll();
#endif
    }

    void replot(replot_in *in, replot_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        SIM::getInstance()->replot(plot);
#endif
    }

    void addCurve(addCurve_in *in, addCurve_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        plot->curveNameMustNotExist(in->name);
        SIM::getInstance()->addCurve(plot, in->type, in->name, in->color, in->style, &in->options);
#endif
    }

    void addCurveTimePoints(addCurveTimePoints_in *in, addCurveTimePoints_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        QCPAbstractPlottable *curve = plot->curveNameMustExist(in->name)->second;
        plot->curveMustBeTime(curve);
        SIM::getInstance()->addCurveTimePoints(plot, in->name, in->x, in->y);
#endif
    }

    void addCurveXYPoints(addCurveXYPoints_in *in, addCurveXYPoints_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        QCPAbstractPlottable *curve = plot->curveNameMustExist(in->name)->second;
        plot->curveMustBeXY(curve);
        SIM::getInstance()->addCurveXYPoints(plot, in->name, in->t, in->x, in->y);
#endif
    }

    void clearCurve(clearCurve_in *in, clearCurve_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        plot->curveNameMustExist(in->name);
        SIM::getInstance()->clearCurve(plot, in->name);
#endif
    }

    void removeCurve(removeCurve_in *in, removeCurve_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        plot->curveNameMustExist(in->name);
        SIM::getInstance()->removeCurve(plot, in->name);
#endif
    }

    void setPlotRanges(setPlotRanges_in *in, setPlotRanges_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        SIM::getInstance()->setPlotRanges(plot, in->xmin, in->xmax, in->ymin, in->ymax);
#endif
    }

    void setPlotXRange(setPlotXRange_in *in, setPlotXRange_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        SIM::getInstance()->setPlotXRange(plot, in->xmin, in->xmax);
#endif
    }

    void setPlotYRange(setPlotYRange_in *in, setPlotYRange_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        SIM::getInstance()->setPlotYRange(plot, in->ymin, in->ymax);
#endif
    }

    void growPlotRanges(growPlotRanges_in *in, growPlotRanges_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        SIM::getInstance()->growPlotRanges(plot, in->xmin, in->xmax, in->ymin, in->ymax);
#endif
    }

    void growPlotXRange(growPlotXRange_in *in, growPlotXRange_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        SIM::getInstance()->growPlotXRange(plot, in->xmin, in->xmax);
#endif
    }

    void growPlotYRange(growPlotYRange_in *in, growPlotYRange_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        SIM::getInstance()->growPlotYRange(plot, in->ymin, in->ymax);
#endif
    }

    void setPlotLabels(setPlotLabels_in *in, setPlotLabels_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        SIM::getInstance()->setPlotLabels(plot, in->x, in->y);
#endif
    }

    void setPlotXLabel(setPlotXLabel_in *in, setPlotXLabel_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        SIM::getInstance()->setPlotXLabel(plot, in->label);
#endif
    }

    void setPlotYLabel(setPlotYLabel_in *in, setPlotYLabel_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        SIM::getInstance()->setPlotYLabel(plot, in->label);
#endif
    }

    void rescaleAxes(rescaleAxes_in *in, rescaleAxes_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        plot->curveNameMustExist(in->name);
        SIM::getInstance()->rescaleAxes(plot, in->name, in->onlyEnlargeX, in->onlyEnlargeY);
#endif
    }

    void rescaleAxesAll(rescaleAxesAll_in *in, rescaleAxesAll_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        SIM::getInstance()->rescaleAxesAll(plot, in->onlyEnlargeX, in->onlyEnlargeY);
#endif
    }

    void setMouseOptions(setMouseOptions_in *in, setMouseOptions_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        SIM::getInstance()->setMouseOptions(plot, in->panX, in->panY, in->zoomX, in->zoomY);
#endif
    }

    void setLegendVisibility(setLegendVisibility_in *in, setLegendVisibility_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        SIM::getInstance()->setLegendVisibility(plot, in->visible);
#endif
    }

    void getCurveData(getCurveData_in *in, getCurveData_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        plot->getCurveData(in->name, out->x, out->x, out->y);
#endif
    }

    void clearTable(clearTable_in *in, clearTable_out *out)
    {
#if WIDGET_TABLE
        Table *table = getWidget<Table>(in->handle, in->id, "table");
        SIM::getInstance()->clearTable(table, in->suppressEvents);
#endif
    }

    void setRowCount(setRowCount_in *in, setRowCount_out *out)
    {
#if WIDGET_TABLE
        Table *table = getWidget<Table>(in->handle, in->id, "table");
        SIM::getInstance()->setRowCount(table, in->count, in->suppressEvents);
#endif
    }

    void setColumnCount(setColumnCount_in *in, setColumnCount_out *out)
    {
        Widget *widget = getWidget(in->handle, in->id);
#if WIDGET_TABLE
        if(Table *table = dynamic_cast<Table*>(widget))
        {
            SIM::getInstance()->setColumnCountTable(table, in->count, in->suppressEvents);
            return;
        }
#endif
#if WIDGET_TREE
        if(Tree *tree = dynamic_cast<Tree*>(widget))
        {
            SIM::getInstance()->setColumnCountTree(tree, in->count, in->suppressEvents);
            return;
        }
#endif
#if WIDGET_TABLE && WIDGET_TREE
        throw std::runtime_error("invalid widget type. expected table or tree.");
#elif WIDGET_TABLE
        throw std::runtime_error("invalid widget type. expected table.");
#elif WIDGET_TREE
        throw std::runtime_error("invalid widget type. expected tree.");
#endif
    }

    void setItem(setItem_in *in, setItem_out *out)
    {
#if WIDGET_TABLE
        Table *table = getWidget<Table>(in->handle, in->id, "table");
        SIM::getInstance()->setItem(table, in->row, in->column, in->text, in->suppressEvents);
#endif
    }

    void setItemImage(setItemImage_in *in, setItemImage_out *out)
    {
#if WIDGET_TABLE
        Table *table = getWidget<Table>(in->handle, in->id, "table");
        SIM::getInstance()->setItemImage(table, in->row, in->column, in->data, in->width, in->height, in->suppressEvents);
#endif
    }

    void setItems(setItems_in *in, setItems_out *out)
    {
#if WIDGET_TABLE
        Table *table = getWidget<Table>(in->handle, in->id, "table");
        SIM::getInstance()->setItems(table, in->data, in->suppressEvents);
#endif
    }

    void getRowCount(getRowCount_in *in, getRowCount_out *out)
    {
#if WIDGET_TABLE
        Table *table = getWidget<Table>(in->handle, in->id, "table");
        out->count = table->getRowCount();
#endif
    }

    void getColumnCount(getColumnCount_in *in, getColumnCount_out *out)
    {
        Widget *widget = getWidget(in->handle, in->id);
#if WIDGET_TABLE
        if(Table *table = dynamic_cast<Table*>(widget))
        {
            out->count = table->getColumnCount();
            return;
        }
#endif
#if WIDGET_TREE
        if(Tree *tree = dynamic_cast<Tree*>(widget))
        {
            out->count = tree->getColumnCount();
            return;
        }
#endif
#if WIDGET_TABLE && WIDGET_TREE
        throw std::runtime_error("invalid widget type. expected table or tree.");
#elif WIDGET_TABLE
        throw std::runtime_error("invalid widget type. expected table.");
#elif WIDGET_TREE
        throw std::runtime_error("invalid widget type. expected tree.");
#endif
    }

    void getItem(getItem_in *in, getItem_out *out)
    {
#if WIDGET_TABLE
        Table *table = getWidget<Table>(in->handle, in->id, "table");
        out->text = table->getItem(in->row, in->column);
#endif
    }

    void setRowHeaderText(setRowHeaderText_in *in, setRowHeaderText_out *out)
    {
#if WIDGET_TABLE
        Table *table = getWidget<Table>(in->handle, in->id, "table");
        SIM::getInstance()->setRowHeaderText(table, in->row, in->text);
#endif
    }

    void setColumnHeaderText(setColumnHeaderText_in *in, setColumnHeaderText_out *out)
    {
        Widget *widget = getWidget(in->handle, in->id);
#if WIDGET_TABLE
        if(Table *table = dynamic_cast<Table*>(widget))
        {
            SIM::getInstance()->setColumnHeaderTextTable(table, in->column, in->text);
            return;
        }
#endif
#if WIDGET_TREE
        if(Tree *tree = dynamic_cast<Tree*>(widget))
        {
            SIM::getInstance()->setColumnHeaderTextTree(tree, in->column, in->text);
            return;
        }
#endif
#if WIDGET_TABLE && WIDGET_TREE
        throw std::runtime_error("invalid widget type. expected table or tree.");
#elif WIDGET_TABLE
        throw std::runtime_error("invalid widget type. expected table.");
#elif WIDGET_TREE
        throw std::runtime_error("invalid widget type. expected tree.");
#endif
    }

    void setItemEditable(setItemEditable_in *in, setItemEditable_out *out)
    {
#if WIDGET_TABLE
        Table *table = getWidget<Table>(in->handle, in->id, "table");
        SIM::getInstance()->setItemEditable(table, in->row, in->column, in->editable);
#endif
    }

    void saveState(saveState_in *in, saveState_out *out)
    {
        Widget *widget = getWidget(in->handle, in->id);
#if WIDGET_TABLE
        if(Table *table = dynamic_cast<Table*>(widget))
        {
            out->state = table->saveState();
            return;
        }
#endif
#if WIDGET_TREE
        if(Tree *tree = dynamic_cast<Tree*>(widget))
        {
            out->state = tree->saveState();
            return;
        }
#endif
#if WIDGET_TABLE && WIDGET_TREE
        throw std::runtime_error("invalid widget type. expected table or tree.");
#elif WIDGET_TABLE
        throw std::runtime_error("invalid widget type. expected table.");
#elif WIDGET_TREE
        throw std::runtime_error("invalid widget type. expected tree.");
#endif
    }

    void restoreState(restoreState_in *in, restoreState_out *out)
    {
        Widget *widget = getWidget(in->handle, in->id);
#if WIDGET_TABLE
        if(Table *table = dynamic_cast<Table*>(widget))
        {
            SIM::getInstance()->restoreStateTable(table, in->state);
            return;
        }
#endif
#if WIDGET_TREE
        if(Tree *tree = dynamic_cast<Tree*>(widget))
        {
            SIM::getInstance()->restoreStateTree(tree, in->state);
            return;
        }
#endif
#if WIDGET_TABLE && WIDGET_TREE
        throw std::runtime_error("invalid widget type. expected table or tree.");
#elif WIDGET_TABLE
        throw std::runtime_error("invalid widget type. expected table.");
#elif WIDGET_TREE
        throw std::runtime_error("invalid widget type. expected tree.");
#endif
    }

    void setRowHeight(setRowHeight_in *in, setRowHeight_out *out)
    {
#if WIDGET_TABLE
        Table *table = getWidget<Table>(in->handle, in->id, "table");
        SIM::getInstance()->setRowHeight(table, in->row, in->min_size, in->max_size);
#endif
    }

    void setColumnWidth(setColumnWidth_in *in, setColumnWidth_out *out)
    {
        Widget *widget = getWidget(in->handle, in->id);
#if WIDGET_TABLE
        if(Table *table = dynamic_cast<Table*>(widget))
        {
            SIM::getInstance()->setColumnWidthTable(table, in->column, in->min_size, in->max_size);
            return;
        }
#endif
#if WIDGET_TREE
        if(Tree *tree = dynamic_cast<Tree*>(widget))
        {
            SIM::getInstance()->setColumnWidthTree(tree, in->column, in->min_size, in->max_size);
            return;
        }
#endif
#if WIDGET_TABLE && WIDGET_TREE
        throw std::runtime_error("invalid widget type. expected table or tree.");
#elif WIDGET_TABLE
        throw std::runtime_error("invalid widget type. expected table.");
#elif WIDGET_TREE
        throw std::runtime_error("invalid widget type. expected tree.");
#endif
    }

    void setTableSelection(setTableSelection_in *in, setTableSelection_out *out)
    {
#if WIDGET_TABLE
        Table *table = getWidget<Table>(in->handle, in->id, "table");
        SIM::getInstance()->setTableSelection(table, in->row, in->column, in->suppressEvents);
#endif
    }

    void setProgress(setProgress_in *in, setProgress_out *out)
    {
#if WIDGET_PROGRESSBAR
        Progressbar *progressbar = getWidget<Progressbar>(in->handle, in->id, "progressbar");
        SIM::getInstance()->setProgress(progressbar, in->value);
#endif
    }

    void clearTree(clearTree_in *in, clearTree_out *out)
    {
#if WIDGET_TREE
        Tree *tree = getWidget<Tree>(in->handle, in->id, "tree");
        SIM::getInstance()->clearTree(tree, in->suppressEvents);
#endif
    }

    void addTreeItem(addTreeItem_in *in, addTreeItem_out *out)
    {
#if WIDGET_TREE
        Tree *tree = getWidget<Tree>(in->handle, in->id, "tree");
        SIM::getInstance()->addTreeItem(tree, in->item_id, in->parent_id, in->text, in->expanded, in->suppressEvents);
#endif
    }

    void updateTreeItemText(updateTreeItemText_in *in, updateTreeItemText_out *out)
    {
#if WIDGET_TREE
        Tree *tree = getWidget<Tree>(in->handle, in->id, "tree");
        SIM::getInstance()->updateTreeItemText(tree, in->item_id, in->text);
#endif
    }

    void updateTreeItemParent(updateTreeItemParent_in *in, updateTreeItemParent_out *out)
    {
#if WIDGET_TREE
        Tree *tree = getWidget<Tree>(in->handle, in->id, "tree");
        SIM::getInstance()->updateTreeItemParent(tree, in->item_id, in->parent_id, in->suppressEvents);
#endif
    }

    void removeTreeItem(removeTreeItem_in *in, removeTreeItem_out *out)
    {
#if WIDGET_TREE
        Tree *tree = getWidget<Tree>(in->handle, in->id, "tree");
        SIM::getInstance()->removeTreeItem(tree, in->item_id, in->suppressEvents);
#endif
    }

    void setTreeSelection(setTreeSelection_in *in, setTreeSelection_out *out)
    {
#if WIDGET_TREE
        Tree *tree = getWidget<Tree>(in->handle, in->id, "tree");
        SIM::getInstance()->setTreeSelection(tree, in->item_id, in->suppressEvents);
#endif
    }

    void expandAll(expandAll_in *in, expandAll_out *out)
    {
#if WIDGET_TREE
        Tree *tree = getWidget<Tree>(in->handle, in->id, "tree");
        SIM::getInstance()->expandAll(tree, in->suppressEvents);
#endif
    }

    void collapseAll(collapseAll_in *in, collapseAll_out *out)
    {
#if WIDGET_TREE
        Tree *tree = getWidget<Tree>(in->handle, in->id, "tree");
        SIM::getInstance()->collapseAll(tree, in->suppressEvents);
#endif
    }

    void expandToDepth(expandToDepth_in *in, expandToDepth_out *out)
    {
#if WIDGET_TREE
        Tree *tree = getWidget<Tree>(in->handle, in->id, "tree");
        SIM::getInstance()->expandToDepth(tree, in->depth, in->suppressEvents);
#endif
    }

    void setText(setText_in *in, setText_out *out)
    {
#if WIDGET_TEXTBROWSER
        TextBrowser *textbrowser = getWidget<TextBrowser>(in->handle, in->id, "text-browser");
        SIM::getInstance()->setText(textbrowser, in->text, in->suppressEvents);
#endif
    }

    void appendText(appendText_in *in, appendText_out *out)
    {
#if WIDGET_TEXTBROWSER
        TextBrowser *textbrowser = getWidget<TextBrowser>(in->handle, in->id, "text-browser");
        SIM::getInstance()->appendText(textbrowser, in->text, in->suppressEvents);
#endif
    }

    void setUrl(setUrl_in *in, setUrl_out *out)
    {
#if WIDGET_TEXTBROWSER
        TextBrowser *textbrowser = getWidget<TextBrowser>(in->handle, in->id, "text-browser");
        SIM::getInstance()->setUrl(textbrowser, in->url);
#endif
    }

    void addScene3DNode(addScene3DNode_in *in, addScene3DNode_out *out)
    {
#if WIDGET_SCENE3D
        Scene3D *scene3d = getWidget<Scene3D>(in->handle, in->id, "scene3d");
        if(scene3d->nodeExists(in->nodeId)) throw std::runtime_error("node id already exists");
        if(!scene3d->nodeExists(in->parentNodeId)) throw std::runtime_error("parent node id does not exist");
        if(!scene3d->nodeTypeIsValid(in->type)) throw std::runtime_error("invalid node type");
        SIM::getInstance()->addScene3DNode(scene3d, in->nodeId, in->parentNodeId, in->type);
#endif
    }

    void removeScene3DNode(removeScene3DNode_in *in, removeScene3DNode_out *out)
    {
#if WIDGET_SCENE3D
        Scene3D *scene3d = getWidget<Scene3D>(in->handle, in->id, "scene3d");
        if(!scene3d->nodeExists(in->nodeId)) throw std::runtime_error("invalid node id");
        SIM::getInstance()->removeScene3DNode(scene3d, in->nodeId);
#endif
    }

    void setScene3DNodeEnabled(setScene3DNodeEnabled_in *in, setScene3DNodeEnabled_out *out)
    {
#if WIDGET_SCENE3D
        Scene3D *scene3d = getWidget<Scene3D>(in->handle, in->id, "scene3d");
        if(!scene3d->nodeExists(in->nodeId)) throw std::runtime_error("invalid node id");
        SIM::getInstance()->setScene3DNodeEnabled(scene3d, in->nodeId, in->enabled);
#endif
    }

    void setScene3DNodeIntParam(setScene3DNodeIntParam_in *in, setScene3DNodeIntParam_out *out)
    {
#if WIDGET_SCENE3D
        Scene3D *scene3d = getWidget<Scene3D>(in->handle, in->id, "scene3d");
        if(!scene3d->nodeExists(in->nodeId)) throw std::runtime_error("invalid node id");
        SIM::getInstance()->setScene3DIntParam(scene3d, in->nodeId, in->paramName, in->value);
#endif
    }

    void setScene3DNodeFloatParam(setScene3DNodeFloatParam_in *in, setScene3DNodeFloatParam_out *out)
    {
#if WIDGET_SCENE3D
        Scene3D *scene3d = getWidget<Scene3D>(in->handle, in->id, "scene3d");
        if(!scene3d->nodeExists(in->nodeId)) throw std::runtime_error("invalid node id");
        SIM::getInstance()->setScene3DFloatParam(scene3d, in->nodeId, in->paramName, in->value);
#endif
    }

    void setScene3DNodeStringParam(setScene3DNodeStringParam_in *in, setScene3DNodeStringParam_out *out)
    {
#if WIDGET_SCENE3D
        Scene3D *scene3d = getWidget<Scene3D>(in->handle, in->id, "scene3d");
        if(!scene3d->nodeExists(in->nodeId)) throw std::runtime_error("invalid node id");
        SIM::getInstance()->setScene3DStringParam(scene3d, in->nodeId, in->paramName, in->value);
#endif
    }

    void setScene3DNodeVector2Param(setScene3DNodeVector2Param_in *in, setScene3DNodeVector2Param_out *out)
    {
#if WIDGET_SCENE3D
        Scene3D *scene3d = getWidget<Scene3D>(in->handle, in->id, "scene3d");
        if(!scene3d->nodeExists(in->nodeId)) throw std::runtime_error("invalid node id");
        SIM::getInstance()->setScene3DVector2Param(scene3d, in->nodeId, in->paramName, in->x, in->y);
#endif
    }

    void setScene3DNodeVector3Param(setScene3DNodeVector3Param_in *in, setScene3DNodeVector3Param_out *out)
    {
#if WIDGET_SCENE3D
        Scene3D *scene3d = getWidget<Scene3D>(in->handle, in->id, "scene3d");
        if(!scene3d->nodeExists(in->nodeId)) throw std::runtime_error("invalid node id");
        SIM::getInstance()->setScene3DVector3Param(scene3d, in->nodeId, in->paramName, in->x, in->y, in->z);
#endif
    }

    void setScene3DNodeVector4Param(setScene3DNodeVector4Param_in *in, setScene3DNodeVector4Param_out *out)
    {
#if WIDGET_SCENE3D
        Scene3D *scene3d = getWidget<Scene3D>(in->handle, in->id, "scene3d");
        if(!scene3d->nodeExists(in->nodeId)) throw std::runtime_error("invalid node id");
        SIM::getInstance()->setScene3DVector4Param(scene3d, in->nodeId, in->paramName, in->x, in->y, in->z, in->w);
#endif
    }

    void svgLoadFile(svgLoadFile_in *in, svgLoadFile_out *out)
    {
#if WIDGET_SVG
        SVG *svg = getWidget<SVG>(in->handle, in->id, "svg");
        QString file = QString::fromStdString(in->file);
        SIM::getInstance()->svgLoadFile(svg, file);
#endif
    }

    void svgLoadData(svgLoadData_in *in, svgLoadData_out *out)
    {
#if WIDGET_SVG
        SVG *svg = getWidget<SVG>(in->handle, in->id, "svg");
        QByteArray data(in->data.data(), in->data.size());
        SIM::getInstance()->svgLoadData(svg, data);
#endif
    }

    void setProperties(setProperties_in *in, setProperties_out *out)
    {
#if WIDGET_PROPERTIES
        Properties *properties = getWidget<Properties>(in->handle, in->id, "properties");
        SIM::getInstance()->setProperties(properties, in->pnames, in->ptypes, in->pvalues, in->pflags, in->pdisplayk, in->pdisplayv, in->suppressEvents);
#endif
    }

    void setPropertiesRows(setPropertiesRows_in *in, setPropertiesRows_out *out)
    {
#if WIDGET_PROPERTIES
        Properties *properties = getWidget<Properties>(in->handle, in->id, "properties");
        SIM::getInstance()->setPropertiesRows(properties, in->rows, in->pnames, in->ptypes, in->pvalues, in->pflags, in->pdisplayk, in->pdisplayv, in->suppressEvents);
#endif
    }

    void setPropertiesSelection(setPropertiesSelection_in *in, setPropertiesSelection_out *out)
    {
#if WIDGET_PROPERTIES
        Properties *properties = getWidget<Properties>(in->handle, in->id, "properties");
        SIM::getInstance()->setPropertiesSelection(properties, in->row, in->suppressEvents);
#endif
    }

    void setPropertiesContextMenu(setPropertiesContextMenu_in *in, setPropertiesContextMenu_out *out)
    {
#if WIDGET_PROPERTIES
        Properties *properties = getWidget<Properties>(in->handle, in->id, "properties");
        SIM::getInstance()->setPropertiesContextMenu(properties, in->keys, in->titles);
#endif
    }

    void setPropertiesState(setPropertiesState_in *in, setPropertiesState_out *out)
    {
#if WIDGET_PROPERTIES
        Properties *properties = getWidget<Properties>(in->handle, in->id, "properties");
        SIM::getInstance()->setPropertiesState(properties, in->state);
#endif
    }

    void getPropertiesState(getPropertiesState_in *in, getPropertiesState_out *out)
    {
#if WIDGET_PROPERTIES
        Properties *properties = getWidget<Properties>(in->handle, in->id, "properties");
        SIM::getInstance()->getPropertiesState(properties, &out->state);
#endif
    }

    void supportedImageFormats(supportedImageFormats_in *in, supportedImageFormats_out *out)
    {
        QList<QByteArray> fmts = QImageReader::supportedImageFormats();
        std::string sep{""}, s;
        for(const auto &fmt : fmts)
        {
            out->formatList.push_back(fmt.data());
            if(in->separator)
            {
                s += sep + fmt.data();
                sep = *in->separator;
            }
        }
        if(in->separator)
            out->formatListStr = s;
    }

    void getKeyboardModifiers(getKeyboardModifiers_in *in, getKeyboardModifiers_out *out)
    {
        auto m = static_cast<QGuiApplication*>(QApplication::instance())->keyboardModifiers();
        out->m.shift = m & Qt::ShiftModifier;
        out->m.control = m & Qt::ControlModifier;
        out->m.alt = m & Qt::AltModifier;
    }

    void qtVersion(qtVersion_in *in, qtVersion_out *out)
    {
        auto v = QLibraryInfo::version().segments();
        out->version = std::vector<int>(v.constBegin(), v.constEnd());
    }

    void setClipboardText(setClipboardText_in *in, setClipboardText_out *out)
    {
        SIM::getInstance()->setClipboardText(QString::fromStdString(in->text));
    }

    void hash(hash_in *in, hash_out *out)
    {
        QByteArray ba = QByteArray::fromStdString(in->data);
        QByteArray hash;
#define ADD_HASHING_ALGORITHM(algo_name) else if(in->algorithm == #algo_name) hash = QCryptographicHash::hash(ba, QCryptographicHash::algo_name)
        if(0);
        ADD_HASHING_ALGORITHM(Md4);
        ADD_HASHING_ALGORITHM(Md5);
        ADD_HASHING_ALGORITHM(Sha1);
        ADD_HASHING_ALGORITHM(Sha224);
        ADD_HASHING_ALGORITHM(Sha256);
        ADD_HASHING_ALGORITHM(Sha384);
        ADD_HASHING_ALGORITHM(Sha512);
        ADD_HASHING_ALGORITHM(Sha3_224);
        ADD_HASHING_ALGORITHM(Sha3_256);
        ADD_HASHING_ALGORITHM(Sha3_384);
        ADD_HASHING_ALGORITHM(Sha3_512);
        ADD_HASHING_ALGORITHM(Keccak_224);
        ADD_HASHING_ALGORITHM(Keccak_256);
        ADD_HASHING_ALGORITHM(Keccak_384);
        ADD_HASHING_ALGORITHM(Keccak_512);
        else if(in->algorithm == "Crc_32")
        {
            static const quint32 crc32Table[256] = {
                0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
                0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
                0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
                0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
                0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
                0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
                0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
                0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
                0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
                0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
                0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
                0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
                0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
                0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
                0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
                0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
                0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
                0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
                0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
                0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
                0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
                0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
                0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
                0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
                0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
                0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
                0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
                0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
                0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
                0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
                0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
                0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
                0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
                0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
                0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
                0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
                0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
                0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
                0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
                0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
                0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
                0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
                0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
                0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
                0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
                0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
                0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
                0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
                0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
                0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
                0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
                0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
                0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
                0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
                0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
                0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
                0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
                0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
                0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
                0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
                0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
                0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
                0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
                0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D,
            };
            hash.resize(4);
            quint32 crc = 0xFFFFFFFF;
            for(quint8 b : ba)
                crc = (crc >> 8) ^ crc32Table[(crc ^ b) & 0xFF];
            crc = ~crc;
            hash[0] = (crc >> 24) & 0xFF;
            hash[1] = (crc >> 16) & 0xFF;
            hash[2] = (crc >> 8)  & 0xFF;
            hash[3] = crc & 0xFF;
        }
        else
            throw std::runtime_error("unsupported hashing algorithm");
        out->hash = hash.toHex().toStdString();
    }

private:
    sim::Handles<Proxy*> handles{"UI"};
};

SIM_UI_PLUGIN(Plugin)
#include "stubsPlusPlus.cpp"
