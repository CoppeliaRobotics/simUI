#include "Window.h"
#include "Widget.h"

#include "XMLUtils.h"

#include "UI.h"

#include "stubs.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstdlib>

#include <QDialog>

std::set<Window *> Window::windows;

WindowWidget::WindowWidget()
    : Widget("window")
{
}

QWidget * WindowWidget::createQtWidget(Proxy *proxy, UI *ui, QWidget *parent)
{
    throw std::runtime_error("WindowWidget cannot be used");
}

Window::Window()
    : qwidget(NULL),
      qwidget_geometry_saved(false),
      visibility_state(true),
      proxy(NULL)
{
    sim::addLog(sim_verbosity_debug, __FUNC__);
}

Window::~Window()
{
    Window::windows.erase(this);

    sim::addLog(sim_verbosity_debug, __FUNC__);
}

void Window::parse(std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    std::string tag(e->Value());
    if(tag != "ui")
    {
        throw std::range_error("root element must be <ui>");
    }

    title = xmlutils::getAttrStr(e, "title", "Custom UI");

    enabled = xmlutils::getAttrBool(e, "enabled", true);

    resizable = xmlutils::getAttrBool(e, "resizable", false);

    closeable = xmlutils::getAttrBool(e, "closeable", false);

    modal = xmlutils::getAttrBool(e, "modal", false);

    onclose = xmlutils::getAttrStr(e, "on-close", "");

    style = xmlutils::getAttrStr(e, "style", "");

    placement = xmlutils::getAttrStr(e, "placement", "center");

    std::vector<int> position = xmlutils::getAttrIntV(e, "position", "50,50", 2, 2, ",");
    qwidget_pos.setX(position[0]);
    qwidget_pos.setY(position[1]);

    std::vector<int> size = xmlutils::getAttrIntV(e, "size", "-1,-1", 2, 2, ",");
    qwidget_size.setWidth(size[0]);
    qwidget_size.setHeight(size[1]);

    activate = xmlutils::getAttrBool(e, "activate", true);

    WindowWidget dummyWidget;
    LayoutWidget::parse(&dummyWidget, &dummyWidget, widgets, e);

    windows.insert(this);
}

bool Window::exists(Window *w)
{
    return Window::windows.find(w) != Window::windows.end();
}

#include <QCloseEvent>

class QDialog2 : public QDialog
{
protected:
    Window *window;

public:
    QDialog2(Window *window_, QWidget *parent = 0)
        : QDialog(parent),
          window(window_)
    {
    }

    virtual ~QDialog2()
    {
        // XXX: `delete window->proxy;` would indirectly delete also `window`!
        // therefor the next line (`window->proxy = NULL`) would be an use after free
        auto tmp = window->proxy;
        window->proxy = NULL;
        delete tmp;
    }

    void keyPressEvent(QKeyEvent * event)
    {
        if(event->key() == Qt::Key_Escape)
        {
            if(window->onclose != "")
            {
                event->ignore();
                UI::getInstance()->windowClose(window);
            }
        }
        else QDialog::keyPressEvent(event);
    }

    virtual void closeEvent(QCloseEvent * event)
    {
        if(window->onclose != "")
        {
            event->ignore();
            UI::getInstance()->windowClose(window);
        }
        else
        {
            event->accept();
        }
    }
};

QWidget * Window::createQtWidget(Proxy *proxy, UI *ui, QWidget *parent)
{
    QDialog *window = new QDialog2(this, parent);
    qwidget = window;
    window->setStyleSheet(QString::fromStdString(style));
    LayoutWidget::createQtWidget(proxy, ui, window);
    window->setWindowTitle(QString::fromStdString(title));
    Qt::WindowFlags flags = Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint;
#ifdef MAC_SIM
    flags |= Qt::Tool;
#else
#ifdef LIN_SIM
    flags |= Qt::Dialog; // Qt::Window doesn't stay above CoppeliaSim's main window since Qt 5.9
#else
    flags |= Qt::Dialog;
#endif
#endif
    if(resizable) flags |= Qt::WindowMaximizeButtonHint;
    else flags |= Qt::MSWindowsFixedSizeDialogHint;
    if(closeable) flags |= Qt::WindowCloseButtonHint;
    window->setWindowFlags(flags);
    window->setAttribute(Qt::WA_MacAlwaysShowToolWindow);
    window->setModal(modal);
    //window->setAttribute(Qt::WA_DeleteOnClose);
    if(!activate) window->setAttribute(Qt::WA_ShowWithoutActivating);
    if(qwidget_size.isValid())
        resize(qwidget_size);
    window->setEnabled(enabled);
    window->show();
#if defined(LIN_SIM) || defined(MAC_SIM)
    if(!resizable) window->setFixedSize(window->size());
#endif
    if(placement == "relative")
    {
        QPoint p;
        if(qwidget_pos.x() >= 0)
            p.setX(parent->window()->frameGeometry().left() + qwidget_pos.x());
        else
            p.setX(parent->window()->frameGeometry().right() - window->rect().right() + qwidget_pos.x());
        if(qwidget_pos.y() >= 0)
            p.setY(parent->window()->frameGeometry().top() + qwidget_pos.y());
        else
            p.setY(parent->window()->frameGeometry().bottom() - window->rect().bottom() + qwidget_pos.y());
        move(p);
    }
    else if(placement == "absolute")
    {
        move(qwidget_pos);
    }
    this->proxy = proxy;
    return window;
}

std::string Window::str()
{
    std::stringstream ss;
    ss << "Window[" << this << "]";
    return ss.str();
}

void Window::adjustSize()
{
    qwidget->adjustSize();
}

void Window::hide()
{
    qwidget_pos = pos();
    qwidget_size = size();
    qwidget_geometry_saved = true;
    visibility_state = false;
    qwidget->hide();
}

void Window::show()
{
    if(qwidget_geometry_saved)
    {
        move(qwidget_pos);
        resize(qwidget_size);
    }
    visibility_state = true;
    qwidget->show();
}

QPoint Window::pos()
{
    return qwidget->geometry().topLeft();
}

void Window::move(const QPoint &p)
{
    qwidget->setGeometry(p.x(), p.y(), qwidget->width(), qwidget->height());
}

void Window::move(int x, int y)
{
    qwidget->setGeometry(x, y, qwidget->width(), qwidget->height());
}

QSize Window::size()
{
    return qwidget->size();
}

void Window::resize(const QSize &s)
{
    qwidget->resize(s);
}

void Window::resize(int w, int h)
{
    qwidget->resize(w, h);
}

void Window::setTitle(std::string title)
{
    QDialog *dialog = static_cast<QDialog*>(getQWidget());
    dialog->setWindowTitle(QString::fromStdString(title));
}

std::string Window::getTitle()
{
    QDialog *dialog = static_cast<QDialog*>(getQWidget());
    return dialog->windowTitle().toStdString();
}

void Window::setEnabled(bool enabled)
{
    QDialog *dialog = static_cast<QDialog*>(getQWidget());
    return dialog->setEnabled(enabled);
}

void Window::onSceneChange(int oldSceneID, int newSceneID)
{
    int mySceneID = proxy->getSceneID();

    sim::addLog(sim_verbosity_debug, "onSceneChange: oldSceneID=%d, newSceneID=%d (mySceneID=%d)", oldSceneID, newSceneID, mySceneID);

    switch(proxy->getScriptType())
    {
    case sim_scripttype_addon:
    case sim_scripttype_sandbox:
        // for these script types, don't hide UI when switching scene
        return;
    }

    if(oldSceneID == mySceneID && newSceneID != mySceneID)
    {
        sim::addLog(sim_verbosity_debug, "hidden");

        bool saved_visibility_state = visibility_state;
        hide();
        visibility_state = saved_visibility_state;
    }

    if(oldSceneID != mySceneID && newSceneID == mySceneID)
    {
        sim::addLog(sim_verbosity_debug, "shown");

        if(visibility_state)
            show();
    }
}

