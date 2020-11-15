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

#include "tinyxml2.h"

#include "simPlusPlus/Plugin.h"
#include "plugin.h"
#include "config.h"
#include "stubs.h"
#include "Proxy.h"
#include "UIFunctions.h"
#include "UIProxy.h"
#include "widgets/all.h"

#ifdef ENABLE_SIGNAL_SPY
#include "signal_spy.h"
#endif

using namespace simExtCustomUI;

class Plugin : public sim::Plugin
{
public:
    void onStart()
    {
        oldSceneID = sim::getInt32Parameter(sim_intparam_scene_unique_id);

        if(simGetBooleanParameter(sim_boolparam_headless) > 0)
            throw std::runtime_error("doesn't work in headless mode");

        if(!registerScriptStuff())
            throw std::runtime_error("failed to register script stuff");

        setExtVersion("Custom User-Interface Plugin");
        setBuildDate(BUILD_DATE);

#if defined(ENABLE_SIGNAL_SPY) && !defined(NDEBUG)
        SignalSpy::start();
#endif

        UIProxy::getInstance(); // construct UIProxy here (UI thread)
    }

    void onEnd()
    {
        Proxy::destroyAllObjectsFromUIThread();

        UIFunctions::destroyInstance();
        UIProxy::destroyInstance();

        UI_THREAD = NULL;
        SIM_THREAD = NULL;
    }

    void onFirstInstancePass(const sim::InstancePassFlags &flags)
    {
        UIFunctions::getInstance(); // construct UIFunctions here (SIM thread)
    }

    void onInstanceSwitch(int sceneID)
    {
        Proxy::sceneChange(oldSceneID, sceneID);
        oldSceneID = sceneID;
    }

    void onSimulationEnded()
    {
        Proxy::destroyTransientObjects();
    }

    void msgBox(msgBox_in *in, msgBox_out *out)
    {
        sim::addLog(sim_verbosity_debug, "[enter]");
        int result;
        // this function is called also from the C API: always run it in the correct thread
        if(QThread::currentThreadId() == UI_THREAD)
            UIProxy::getInstance()->onMsgBox(in->type, in->buttons, in->title, in->message, &result);
        else
            UIFunctions::getInstance()->msgBox(in->type, in->buttons, in->title, in->message, &result);
        out->result = result;
        sim::addLog(sim_verbosity_debug, "[leave]");
    }

    void fileDialog(fileDialog_in *in, fileDialog_out *out)
    {
        sim::addLog(sim_verbosity_debug, "[enter]");
        std::vector<std::string> result;
        // this function is called also from the C API: always run it in the correct thread
        if(QThread::currentThreadId() == UI_THREAD)
            UIProxy::getInstance()->onFileDialog(in->type, in->title, in->startPath, in->initName, in->extName, in->ext, in->native, &result);
        else
            UIFunctions::getInstance()->fileDialog(in->type, in->title, in->startPath, in->initName, in->extName, in->ext, in->native, &result);
        for(auto x : result) out->result.push_back(x);
        sim::addLog(sim_verbosity_debug, "[leave]");
    }

    void colorDialog(colorDialog_in *in, colorDialog_out *out)
    {
        sim::addLog(sim_verbosity_debug, "[enter]");
        // this function is called also from the C API: always run it in the correct thread
        std::vector<float> r;
        if(QThread::currentThreadId() == UI_THREAD)
            UIProxy::getInstance()->onColorDialog(in->initColor, in->title, in->showAlphaChannel, in->native, &r);
        else
            UIFunctions::getInstance()->colorDialog(in->initColor, in->title, in->showAlphaChannel, in->native, &r);
        if(!r.empty()) out->result = r;
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
        bool destroy = false;
        int scriptProperty;
        int objectHandle;
        simGetScriptProperty(in->_scriptID, &scriptProperty, &objectHandle);
#if SIM_PROGRAM_FULL_VERSION_NB <= 4010003
        int scriptType = (scriptProperty | sim_scripttype_threaded) - sim_scripttype_threaded;
#else
        int scriptType = (scriptProperty | sim_scripttype_threaded_old) - sim_scripttype_threaded_old;
#endif
        if(scriptType == sim_scripttype_mainscript || scriptType == sim_scripttype_childscript)
            destroy = true;

        int sceneID = sim::getInt32Parameter(sim_intparam_scene_unique_id);
        sim::addLog(sim_verbosity_debug, "Creating a new Proxy object... (destroy at simulation end = %s)", (destroy ? "true" : "false"));
        Proxy *proxy = new Proxy(destroy, sceneID, in->_scriptID, window, widgets);
        out->uiHandle = proxy->getHandle();
        sim::addLog(sim_verbosity_debug, "Proxy %d created in scene %d", proxy->getHandle(), sceneID);

        sim::addLog(sim_verbosity_debug, "call UIFunctions::create() (will emit the create(Proxy*) signal)...");
        UIFunctions::getInstance()->create(proxy); // connected to UIProxy, which
                                // will run code for creating Qt widgets in the UI thread
        sim::addLog(sim_verbosity_debug, "[leave]");
    }

    void destroy(destroy_in *in, destroy_out *out)
    {
        ASSERT_THREAD(!UI);
        sim::addLog(sim_verbosity_debug, "[enter]");

        Proxy *proxy = Proxy::byHandle(in->handle);
        if(!proxy)
        {
            sim::addLog(sim_verbosity_debug, "invalid ui handle: %d", in->handle);

            throw std::runtime_error("invalid ui handle");
        }

        sim::addLog(sim_verbosity_debug, "call UIFunctions::destroy() (will emit the destroy(Proxy*) signal)...");
        UIFunctions::getInstance()->destroy(proxy); // will also delete proxy

        sim::addLog(sim_verbosity_debug, "[leave]");
    }

    Widget* getWidget(int handle, int id)
    {
        Widget *widget = Widget::byId(handle, id);
        if(!widget)
        {
            std::stringstream ss;
            ss << "invalid widget id: " << id;
            throw std::runtime_error(ss.str());
        }
        return widget;
    }

    template<typename T>
    T* getWidget(int handle, int id, const char *widget_type_name)
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
    T* getQWidget(int handle, int id, const char *widget_type_name)
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
        UIFunctions::getInstance()->setStyleSheet(widget, in->styleSheet);
    }

    void setButtonText(setButtonText_in *in, setButtonText_out *out)
    {
#if WIDGET_BUTTON
        ASSERT_THREAD(!UI);
        Button *button = getWidget<Button>(in->handle, in->id, "button");
        UIFunctions::getInstance()->setButtonText(button, in->text);
#endif
    }

    void setButtonPressed(setButtonPressed_in *in, setButtonPressed_out *out)
    {
#if WIDGET_BUTTON
        ASSERT_THREAD(!UI);
        Button *button = getWidget<Button>(in->handle, in->id, "button");
        UIFunctions::getInstance()->setButtonPressed(button, in->pressed);
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
        UIFunctions::getInstance()->setSliderValue(slider, in->value, in->suppressEvents);
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
        UIFunctions::getInstance()->setEditValue(edit, in->value, in->suppressEvents);
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
        UIFunctions::getInstance()->setSpinboxValue(spinbox, in->value, in->suppressEvents);
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
        UIFunctions::getInstance()->setCheckboxValue(checkbox, value, in->suppressEvents);
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
        UIFunctions::getInstance()->setRadiobuttonValue(radiobutton, value, in->suppressEvents);
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
        UIFunctions::getInstance()->setLabelText(label, in->text, in->suppressEvents);
#endif
    }

    void insertComboboxItem(insertComboboxItem_in *in, insertComboboxItem_out *out)
    {
#if WIDGET_COMBOBOX
        ASSERT_THREAD(!UI);
        Combobox *combobox = getWidget<Combobox>(in->handle, in->id, "combobox");
        UIFunctions::getInstance()->insertComboboxItem(combobox, in->index, in->text, in->suppressEvents);
#endif
    }

    void removeComboboxItem(removeComboboxItem_in *in, removeComboboxItem_out *out)
    {
#if WIDGET_COMBOBOX
        ASSERT_THREAD(!UI);
        Combobox *combobox = getWidget<Combobox>(in->handle, in->id, "combobox");
        UIFunctions::getInstance()->removeComboboxItem(combobox, in->index, in->suppressEvents);
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
        UIFunctions::getInstance()->setComboboxItems(combobox, in->items, in->index, in->suppressEvents);
#endif
    }

    void setComboboxSelectedIndex(setComboboxSelectedIndex_in *in, setComboboxSelectedIndex_out *out)
    {
#if WIDGET_COMBOBOX
        ASSERT_THREAD(!UI);
        Combobox *combobox = getWidget<Combobox>(in->handle, in->id, "combobox");
        UIFunctions::getInstance()->setComboboxSelectedIndex(combobox, in->index, in->suppressEvents);
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

    void hide(hide_in *in, hide_out *out)
    {
        ASSERT_THREAD(!UI);
        Proxy *proxy = Proxy::byHandle(in->handle);
        if(!proxy)
            throw std::runtime_error("invalid ui handle");

        UIFunctions::getInstance()->hideWindow(proxy->getWidget());
    }

    void show(show_in *in, show_out *out)
    {
        ASSERT_THREAD(!UI);
        Proxy *proxy = Proxy::byHandle(in->handle);
        if(!proxy)
            throw std::runtime_error("invalid ui handle");

        UIFunctions::getInstance()->showWindow(proxy->getWidget());
    }

    void isVisible(isVisible_in *in, isVisible_out *out)
    {
        Proxy *proxy = Proxy::byHandle(in->handle);
        if(!proxy)
            throw std::runtime_error("invalid ui handle");

        out->visibility = proxy->getWidget()->getQWidget()->isVisible();
    }

    void getPosition(getPosition_in *in, getPosition_out *out)
    {
        Proxy *proxy = Proxy::byHandle(in->handle);
        if(!proxy)
            throw std::runtime_error("invalid ui handle");

        QWidget *window = proxy->getWidget()->getQWidget();
        out->x = window->x();
        out->y = window->y();
    }

    void setPosition(setPosition_in *in, setPosition_out *out)
    {
        ASSERT_THREAD(!UI);
        Proxy *proxy = Proxy::byHandle(in->handle);
        if(!proxy)
            throw std::runtime_error("invalid ui handle");

        Window *window = proxy->getWidget();
        UIFunctions::getInstance()->setPosition(window, in->x, in->y);
    }

    void getSize(getSize_in *in, getSize_out *out)
    {
        Proxy *proxy = Proxy::byHandle(in->handle);
        if(!proxy)
            throw std::runtime_error("invalid ui handle");

        QWidget *window = proxy->getWidget()->getQWidget();
        out->w = window->width();
        out->h = window->height();
    }

    void setSize(setSize_in *in, setSize_out *out)
    {
        ASSERT_THREAD(!UI);
        Proxy *proxy = Proxy::byHandle(in->handle);
        if(!proxy)
            throw std::runtime_error("invalid ui handle");

        Window *window = proxy->getWidget();
        UIFunctions::getInstance()->setSize(window, in->w, in->h);
    }

    void getTitle(getTitle_in *in, getTitle_out *out)
    {
        Proxy *proxy = Proxy::byHandle(in->handle);
        if(!proxy)
            throw std::runtime_error("invalid ui handle");

        QWidget *window = proxy->getWidget()->getQWidget();
        out->title = static_cast<QDialog*>(window)->windowTitle().toStdString();
    }

    void setTitle(setTitle_in *in, setTitle_out *out)
    {
        ASSERT_THREAD(!UI);
        Proxy *proxy = Proxy::byHandle(in->handle);
        if(!proxy)
            throw std::runtime_error("invalid ui handle");

        Window *window = proxy->getWidget();
        UIFunctions::getInstance()->setTitle(window, in->title);
    }

    void setWindowEnabled(setWindowEnabled_in *in, setWindowEnabled_out *out)
    {
        ASSERT_THREAD(!UI);
        Proxy *proxy = Proxy::byHandle(in->handle);
        if(!proxy)
            throw std::runtime_error("invalid ui handle");

        Window *window = proxy->getWidget();
        UIFunctions::getInstance()->setWindowEnabled(window, in->enabled);
    }

    void setImageData(setImageData_in *in, setImageData_out *out)
    {
#if WIDGET_IMAGE
        ASSERT_THREAD(!UI);
        Image *imageWidget = dynamic_cast<Image*>(Widget::byId(in->handle, in->id));
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

        simChar *img = sim::createBuffer(sz);
        std::memcpy(img, in->data.c_str(), sz);
        simInt resolution[2] = {in->width, in->height};
        simTransformImage((simUChar *)img, resolution, 4, NULL, NULL, NULL);

        UIFunctions::getInstance()->setImage(imageWidget, img, in->width, in->height);
#endif
    }

    void setEnabled(setEnabled_in *in, setEnabled_out *out)
    {
        ASSERT_THREAD(!UI);
        Widget *widget = Widget::byId(in->handle, in->id);
        if(!widget)
        {
            std::stringstream ss;
            ss << "invalid widget id: " << in->id;
            throw std::runtime_error(ss.str());
        }

        UIFunctions::getInstance()->setEnabled(widget, in->enabled);
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
        UIFunctions::getInstance()->setCurrentTab(tabs, in->index, in->suppressEvents);
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
        Widget *widget = Widget::byId(in->handle, in->id);
        if(!widget)
        {
            std::stringstream ss;
            ss << "invalid widget id: " << in->id;
            throw std::runtime_error(ss.str());
        }

        UIFunctions::getInstance()->setWidgetVisibility(widget, in->visibility);
    }

    void getCurrentEditWidget(getCurrentEditWidget_in *in, getCurrentEditWidget_out *out)
    {
#if WIDGET_EDIT
        Proxy *proxy = Proxy::byHandle(in->handle);
        if(!proxy)
            throw std::runtime_error("invalid ui handle");
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
        UIFunctions::getInstance()->replot(plot);
#endif
    }

    void addCurve(addCurve_in *in, addCurve_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        plot->curveNameMustNotExist(in->name);
        UIFunctions::getInstance()->addCurve(plot, in->type, in->name, in->color, in->style, &in->options);
#endif
    }

    void addCurveTimePoints(addCurveTimePoints_in *in, addCurveTimePoints_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        QCPAbstractPlottable *curve = plot->curveNameMustExist(in->name)->second;
        plot->curveMustBeTime(curve);
        UIFunctions::getInstance()->addCurveTimePoints(plot, in->name, in->x, in->y);
#endif
    }

    void addCurveXYPoints(addCurveXYPoints_in *in, addCurveXYPoints_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        QCPAbstractPlottable *curve = plot->curveNameMustExist(in->name)->second;
        plot->curveMustBeXY(curve);
        UIFunctions::getInstance()->addCurveXYPoints(plot, in->name, in->t, in->x, in->y);
#endif
    }

    void clearCurve(clearCurve_in *in, clearCurve_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        plot->curveNameMustExist(in->name);
        UIFunctions::getInstance()->clearCurve(plot, in->name);
#endif
    }

    void removeCurve(removeCurve_in *in, removeCurve_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        plot->curveNameMustExist(in->name);
        UIFunctions::getInstance()->removeCurve(plot, in->name);
#endif
    }

    void setPlotRanges(setPlotRanges_in *in, setPlotRanges_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        UIFunctions::getInstance()->setPlotRanges(plot, in->xmin, in->xmax, in->ymin, in->ymax);
#endif
    }

    void setPlotXRange(setPlotXRange_in *in, setPlotXRange_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        UIFunctions::getInstance()->setPlotXRange(plot, in->xmin, in->xmax);
#endif
    }

    void setPlotYRange(setPlotYRange_in *in, setPlotYRange_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        UIFunctions::getInstance()->setPlotYRange(plot, in->ymin, in->ymax);
#endif
    }

    void growPlotRanges(growPlotRanges_in *in, growPlotRanges_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        UIFunctions::getInstance()->growPlotRanges(plot, in->xmin, in->xmax, in->ymin, in->ymax);
#endif
    }

    void growPlotXRange(growPlotXRange_in *in, growPlotXRange_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        UIFunctions::getInstance()->growPlotXRange(plot, in->xmin, in->xmax);
#endif
    }

    void growPlotYRange(growPlotYRange_in *in, growPlotYRange_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        UIFunctions::getInstance()->growPlotYRange(plot, in->ymin, in->ymax);
#endif
    }

    void setPlotLabels(setPlotLabels_in *in, setPlotLabels_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        UIFunctions::getInstance()->setPlotLabels(plot, in->x, in->y);
#endif
    }

    void setPlotXLabel(setPlotXLabel_in *in, setPlotXLabel_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        UIFunctions::getInstance()->setPlotXLabel(plot, in->label);
#endif
    }

    void setPlotYLabel(setPlotYLabel_in *in, setPlotYLabel_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        UIFunctions::getInstance()->setPlotYLabel(plot, in->label);
#endif
    }

    void rescaleAxes(rescaleAxes_in *in, rescaleAxes_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        plot->curveNameMustExist(in->name);
        UIFunctions::getInstance()->rescaleAxes(plot, in->name, in->onlyEnlargeX, in->onlyEnlargeY);
#endif
    }

    void rescaleAxesAll(rescaleAxesAll_in *in, rescaleAxesAll_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        UIFunctions::getInstance()->rescaleAxesAll(plot, in->onlyEnlargeX, in->onlyEnlargeY);
#endif
    }

    void setMouseOptions(setMouseOptions_in *in, setMouseOptions_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        UIFunctions::getInstance()->setMouseOptions(plot, in->panX, in->panY, in->zoomX, in->zoomY);
#endif
    }

    void setLegendVisibility(setLegendVisibility_in *in, setLegendVisibility_out *out)
    {
#if WIDGET_PLOT
        Plot *plot = getWidget<Plot>(in->handle, in->id, "plot");
        UIFunctions::getInstance()->setLegendVisibility(plot, in->visible);
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
        UIFunctions::getInstance()->clearTable(table, in->suppressEvents);
#endif
    }

    void setRowCount(setRowCount_in *in, setRowCount_out *out)
    {
#if WIDGET_TABLE
        Table *table = getWidget<Table>(in->handle, in->id, "table");
        UIFunctions::getInstance()->setRowCount(table, in->count, in->suppressEvents);
#endif
    }

    void setColumnCount(setColumnCount_in *in, setColumnCount_out *out)
    {
        Widget *widget = getWidget(in->handle, in->id);
#if WIDGET_TABLE
        if(Table *table = dynamic_cast<Table*>(widget))
        {
            UIFunctions::getInstance()->setColumnCountTable(table, in->count, in->suppressEvents);
            return;
        }
#endif
#if WIDGET_TREE
        if(Tree *tree = dynamic_cast<Tree*>(widget))
        {
            UIFunctions::getInstance()->setColumnCountTree(tree, in->count, in->suppressEvents);
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
        UIFunctions::getInstance()->setItem(table, in->row, in->column, in->text, in->suppressEvents);
#endif
    }

    void setItemImage(setItemImage_in *in, setItemImage_out *out)
    {
#if WIDGET_TABLE
        Table *table = getWidget<Table>(in->handle, in->id, "table");
        UIFunctions::getInstance()->setItemImage(table, in->row, in->column, in->data, in->width, in->height, in->suppressEvents);
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
        UIFunctions::getInstance()->setRowHeaderText(table, in->row, in->text);
#endif
    }

    void setColumnHeaderText(setColumnHeaderText_in *in, setColumnHeaderText_out *out)
    {
        Widget *widget = getWidget(in->handle, in->id);
#if WIDGET_TABLE
        if(Table *table = dynamic_cast<Table*>(widget))
        {
            UIFunctions::getInstance()->setColumnHeaderTextTable(table, in->column, in->text);
            return;
        }
#endif
#if WIDGET_TREE
        if(Tree *tree = dynamic_cast<Tree*>(widget))
        {
            UIFunctions::getInstance()->setColumnHeaderTextTree(tree, in->column, in->text);
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
        UIFunctions::getInstance()->setItemEditable(table, in->row, in->column, in->editable);
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
            UIFunctions::getInstance()->restoreStateTable(table, in->state);
            return;
        }
#endif
#if WIDGET_TREE
        if(Tree *tree = dynamic_cast<Tree*>(widget))
        {
            UIFunctions::getInstance()->restoreStateTree(tree, in->state);
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
        UIFunctions::getInstance()->setRowHeight(table, in->row, in->min_size, in->max_size);
#endif
    }

    void setColumnWidth(setColumnWidth_in *in, setColumnWidth_out *out)
    {
        Widget *widget = getWidget(in->handle, in->id);
#if WIDGET_TABLE
        if(Table *table = dynamic_cast<Table*>(widget))
        {
            UIFunctions::getInstance()->setColumnWidthTable(table, in->column, in->min_size, in->max_size);
            return;
        }
#endif
#if WIDGET_TREE
        if(Tree *tree = dynamic_cast<Tree*>(widget))
        {
            UIFunctions::getInstance()->setColumnWidthTree(tree, in->column, in->min_size, in->max_size);
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
        UIFunctions::getInstance()->setTableSelection(table, in->row, in->column, in->suppressEvents);
#endif
    }

    void setProgress(setProgress_in *in, setProgress_out *out)
    {
#if WIDGET_PROGRESSBAR
        Progressbar *progressbar = getWidget<Progressbar>(in->handle, in->id, "progressbar");
        UIFunctions::getInstance()->setProgress(progressbar, in->value);
#endif
    }

    void clearTree(clearTree_in *in, clearTree_out *out)
    {
#if WIDGET_TREE
        Tree *tree = getWidget<Tree>(in->handle, in->id, "tree");
        UIFunctions::getInstance()->clearTree(tree, in->suppressEvents);
#endif
    }

    void addTreeItem(addTreeItem_in *in, addTreeItem_out *out)
    {
#if WIDGET_TREE
        Tree *tree = getWidget<Tree>(in->handle, in->id, "tree");
        UIFunctions::getInstance()->addTreeItem(tree, in->item_id, in->parent_id, in->text, in->expanded, in->suppressEvents);
#endif
    }

    void updateTreeItemText(updateTreeItemText_in *in, updateTreeItemText_out *out)
    {
#if WIDGET_TREE
        Tree *tree = getWidget<Tree>(in->handle, in->id, "tree");
        UIFunctions::getInstance()->updateTreeItemText(tree, in->item_id, in->text);
#endif
    }

    void updateTreeItemParent(updateTreeItemParent_in *in, updateTreeItemParent_out *out)
    {
#if WIDGET_TREE
        Tree *tree = getWidget<Tree>(in->handle, in->id, "tree");
        UIFunctions::getInstance()->updateTreeItemParent(tree, in->item_id, in->parent_id, in->suppressEvents);
#endif
    }

    void removeTreeItem(removeTreeItem_in *in, removeTreeItem_out *out)
    {
#if WIDGET_TREE
        Tree *tree = getWidget<Tree>(in->handle, in->id, "tree");
        UIFunctions::getInstance()->removeTreeItem(tree, in->item_id, in->suppressEvents);
#endif
    }

    void setTreeSelection(setTreeSelection_in *in, setTreeSelection_out *out)
    {
#if WIDGET_TREE
        Tree *tree = getWidget<Tree>(in->handle, in->id, "tree");
        UIFunctions::getInstance()->setTreeSelection(tree, in->item_id, in->suppressEvents);
#endif
    }

    void expandAll(expandAll_in *in, expandAll_out *out)
    {
#if WIDGET_TREE
        Tree *tree = getWidget<Tree>(in->handle, in->id, "tree");
        UIFunctions::getInstance()->expandAll(tree, in->suppressEvents);
#endif
    }

    void collapseAll(collapseAll_in *in, collapseAll_out *out)
    {
#if WIDGET_TREE
        Tree *tree = getWidget<Tree>(in->handle, in->id, "tree");
        UIFunctions::getInstance()->collapseAll(tree, in->suppressEvents);
#endif
    }

    void expandToDepth(expandToDepth_in *in, expandToDepth_out *out)
    {
#if WIDGET_TREE
        Tree *tree = getWidget<Tree>(in->handle, in->id, "tree");
        UIFunctions::getInstance()->expandToDepth(tree, in->depth, in->suppressEvents);
#endif
    }

    void addNode(addNode_in *in, addNode_out *out)
    {
#if WIDGET_DATAFLOW
        Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, "dataflow");
        out->nodeId = dataflow->nextId();
        UIFunctions::getInstance()->addNode(dataflow, out->nodeId, QPoint(in->x, in->y), QString::fromStdString(in->text), in->inlets, in->outlets);
#endif
    }

    void removeNode(removeNode_in *in, removeNode_out *out)
    {
#if WIDGET_DATAFLOW
        Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, "dataflow");
        UIFunctions::getInstance()->removeNode(dataflow, in->nodeId);
#endif
    }

    void setNodeValid(setNodeValid_in *in, setNodeValid_out *out)
    {
#if WIDGET_DATAFLOW
        Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, "dataflow");
        dataflow->getNode(in->nodeId);
        UIFunctions::getInstance()->setNodeValid(dataflow, in->nodeId, in->valid);
#endif
    }

    void isNodeValid(isNodeValid_in *in, isNodeValid_out *out)
    {
#if WIDGET_DATAFLOW
        Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, "dataflow");
        dataflow->getNode(in->nodeId);
        out->valid = dataflow->isNodeValid(in->nodeId);
#endif
    }

    void setNodePos(setNodePos_in *in, setNodePos_out *out)
    {
#if WIDGET_DATAFLOW
        Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, "dataflow");
        dataflow->getNode(in->nodeId);
        UIFunctions::getInstance()->setNodePos(dataflow, in->nodeId, QPoint(in->x, in->y));
#endif
    }

    void getNodePos(getNodePos_in *in, getNodePos_out *out)
    {
#if WIDGET_DATAFLOW
        Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, "dataflow");
        dataflow->getNode(in->nodeId);
        QPoint pos = dataflow->getNodePos(in->nodeId);
        out->x = pos.x();
        out->y = pos.y();
#endif
    }

    void setNodeText(setNodeText_in *in, setNodeText_out *out)
    {
#if WIDGET_DATAFLOW
        Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, "dataflow");
        dataflow->getNode(in->nodeId);
        UIFunctions::getInstance()->setNodeText(dataflow, in->nodeId, QString::fromStdString(in->text));
#endif
    }

    void getNodeText(getNodeText_in *in, getNodeText_out *out)
    {
#if WIDGET_DATAFLOW
        Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, "dataflow");
        dataflow->getNode(in->nodeId);
        out->text = dataflow->getNodeText(in->nodeId);
#endif
    }

    void setNodeInletCount(setNodeInletCount_in *in, setNodeInletCount_out *out)
    {
#if WIDGET_DATAFLOW
        Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, "dataflow");
        dataflow->getNode(in->nodeId);
        UIFunctions::getInstance()->setNodeInletCount(dataflow, in->nodeId, in->count);
#endif
    }

    void getNodeInletCount(getNodeInletCount_in *in, getNodeInletCount_out *out)
    {
#if WIDGET_DATAFLOW
        Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, "dataflow");
        dataflow->getNode(in->nodeId);
        out->count = dataflow->getNodeInletCount(in->nodeId);
#endif
    }

    void setNodeOutletCount(setNodeOutletCount_in *in, setNodeOutletCount_out *out)
    {
#if WIDGET_DATAFLOW
        Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, "dataflow");
        dataflow->getNode(in->nodeId);
        UIFunctions::getInstance()->setNodeOutletCount(dataflow, in->nodeId, in->count);
#endif
    }

    void getNodeOutletCount(getNodeOutletCount_in *in, getNodeOutletCount_out *out)
    {
#if WIDGET_DATAFLOW
        Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, "dataflow");
        dataflow->getNode(in->nodeId);
        out->count = dataflow->getNodeOutletCount(in->nodeId);
#endif
    }

    void addConnection(addConnection_in *in, addConnection_out *out)
    {
#if WIDGET_DATAFLOW
        Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, "dataflow");
        dataflow->getNode(in->srcNodeId);
        dataflow->getNode(in->dstNodeId);
        UIFunctions::getInstance()->addConnection(dataflow, in->srcNodeId, in->srcOutlet, in->dstNodeId, in->dstInlet);
#endif
    }

    void removeConnection(removeConnection_in *in, removeConnection_out *out)
    {
#if WIDGET_DATAFLOW
        Dataflow *dataflow = getWidget<Dataflow>(in->handle, in->id, "dataflow");
        dataflow->getNode(in->srcNodeId);
        dataflow->getNode(in->dstNodeId);
        UIFunctions::getInstance()->removeConnection(dataflow, in->srcNodeId, in->srcOutlet, in->dstNodeId, in->dstInlet);
#endif
    }

    void setText(setText_in *in, setText_out *out)
    {
#if WIDGET_TEXTBROWSER
        TextBrowser *textbrowser = getWidget<TextBrowser>(in->handle, in->id, "text-browser");
        UIFunctions::getInstance()->setText(textbrowser, in->text, in->suppressEvents);
#endif
    }

    void setUrl(setUrl_in *in, setUrl_out *out)
    {
#if WIDGET_TEXTBROWSER
        TextBrowser *textbrowser = getWidget<TextBrowser>(in->handle, in->id, "text-browser");
        UIFunctions::getInstance()->setUrl(textbrowser, in->url);
#endif
    }

    void addScene3DNode(addScene3DNode_in *in, addScene3DNode_out *out)
    {
#if WIDGET_SCENE3D
        Scene3D *scene3d = getWidget<Scene3D>(in->handle, in->id, "scene3d");
        if(scene3d->nodeExists(in->nodeId)) throw std::runtime_error("node id already exists");
        if(!scene3d->nodeExists(in->parentNodeId)) throw std::runtime_error("parent node id does not exist");
        if(!scene3d->nodeTypeIsValid(in->type)) throw std::runtime_error("invalid node type");
        UIFunctions::getInstance()->addScene3DNode(scene3d, in->nodeId, in->parentNodeId, in->type);
#endif
    }

    void removeScene3DNode(removeScene3DNode_in *in, removeScene3DNode_out *out)
    {
#if WIDGET_SCENE3D
        Scene3D *scene3d = getWidget<Scene3D>(in->handle, in->id, "scene3d");
        if(!scene3d->nodeExists(in->nodeId)) throw std::runtime_error("invalid node id");
        UIFunctions::getInstance()->removeScene3DNode(scene3d, in->nodeId);
#endif
    }

    void setScene3DNodeEnabled(setScene3DNodeEnabled_in *in, setScene3DNodeEnabled_out *out)
    {
#if WIDGET_SCENE3D
        Scene3D *scene3d = getWidget<Scene3D>(in->handle, in->id, "scene3d");
        if(!scene3d->nodeExists(in->nodeId)) throw std::runtime_error("invalid node id");
        UIFunctions::getInstance()->setScene3DNodeEnabled(scene3d, in->nodeId, in->enabled);
#endif
    }

    void setScene3DNodeIntParam(setScene3DNodeIntParam_in *in, setScene3DNodeIntParam_out *out)
    {
#if WIDGET_SCENE3D
        Scene3D *scene3d = getWidget<Scene3D>(in->handle, in->id, "scene3d");
        if(!scene3d->nodeExists(in->nodeId)) throw std::runtime_error("invalid node id");
        UIFunctions::getInstance()->setScene3DIntParam(scene3d, in->nodeId, in->paramName, in->value);
#endif
    }

    void setScene3DNodeFloatParam(setScene3DNodeFloatParam_in *in, setScene3DNodeFloatParam_out *out)
    {
#if WIDGET_SCENE3D
        Scene3D *scene3d = getWidget<Scene3D>(in->handle, in->id, "scene3d");
        if(!scene3d->nodeExists(in->nodeId)) throw std::runtime_error("invalid node id");
        UIFunctions::getInstance()->setScene3DFloatParam(scene3d, in->nodeId, in->paramName, in->value);
#endif
    }

    void setScene3DNodeStringParam(setScene3DNodeStringParam_in *in, setScene3DNodeStringParam_out *out)
    {
#if WIDGET_SCENE3D
        Scene3D *scene3d = getWidget<Scene3D>(in->handle, in->id, "scene3d");
        if(!scene3d->nodeExists(in->nodeId)) throw std::runtime_error("invalid node id");
        UIFunctions::getInstance()->setScene3DStringParam(scene3d, in->nodeId, in->paramName, in->value);
#endif
    }

    void setScene3DNodeVector2Param(setScene3DNodeVector2Param_in *in, setScene3DNodeVector2Param_out *out)
    {
#if WIDGET_SCENE3D
        Scene3D *scene3d = getWidget<Scene3D>(in->handle, in->id, "scene3d");
        if(!scene3d->nodeExists(in->nodeId)) throw std::runtime_error("invalid node id");
        UIFunctions::getInstance()->setScene3DVector2Param(scene3d, in->nodeId, in->paramName, in->x, in->y);
#endif
    }

    void setScene3DNodeVector3Param(setScene3DNodeVector3Param_in *in, setScene3DNodeVector3Param_out *out)
    {
#if WIDGET_SCENE3D
        Scene3D *scene3d = getWidget<Scene3D>(in->handle, in->id, "scene3d");
        if(!scene3d->nodeExists(in->nodeId)) throw std::runtime_error("invalid node id");
        UIFunctions::getInstance()->setScene3DVector3Param(scene3d, in->nodeId, in->paramName, in->x, in->y, in->z);
#endif
    }

    void setScene3DNodeVector4Param(setScene3DNodeVector4Param_in *in, setScene3DNodeVector4Param_out *out)
    {
#if WIDGET_SCENE3D
        Scene3D *scene3d = getWidget<Scene3D>(in->handle, in->id, "scene3d");
        if(!scene3d->nodeExists(in->nodeId)) throw std::runtime_error("invalid node id");
        UIFunctions::getInstance()->setScene3DVector4Param(scene3d, in->nodeId, in->paramName, in->x, in->y, in->z, in->w);
#endif
    }

private:
    int oldSceneID = -1;
};

SIM_PLUGIN(PLUGIN_NAME, PLUGIN_VERSION, Plugin)
#include "stubsPlusPlus.cpp"
