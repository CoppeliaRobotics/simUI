#include "Window.h"
#include "Widget.h"

#include "debug.h"

#include "XMLUtils.h"

#include "UIProxy.h"

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

QWidget * WindowWidget::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    throw std::runtime_error("WindowWidget cannot be used");
}

Window::Window()
    : qwidget(NULL),
      qwidget_geometry_saved(false),
      visibility_state(true),
      proxy(NULL)
{
    DBG << std::endl;
}

Window::~Window()
{
    Window::windows.erase(this);

    DBG << std::endl;
}

void Window::parse(std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    std::string tag(e->Value());
    if(tag != "ui")
    {
        throw std::range_error("root element must be <ui>");
    }

    title = xmlutils::getAttrStr(e, "title", "Custom UI");

    resizable = xmlutils::getAttrBool(e, "resizable", false);

    closeable = xmlutils::getAttrBool(e, "closeable", false);

    modal = xmlutils::getAttrBool(e, "modal", false);

    onclose = xmlutils::getAttrStr(e, "onclose", "");

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
        delete window->proxy;
        window->proxy = NULL;
    }

    void keyPressEvent(QKeyEvent * event)
    {
        if(event->key() == Qt::Key_Escape)
        {
            if(window->onclose != "")
            {
                event->ignore();
                UIProxy::getInstance()->windowClose(window);
            }
        }
        else QDialog::keyPressEvent(event);
    }

    virtual void closeEvent(QCloseEvent * event)
    {
        if(window->onclose != "")
        {
            event->ignore();
            UIProxy::getInstance()->windowClose(window);
        }
        else
        {
            event->accept();
        }
    }
};

QWidget * Window::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QDialog *window = new QDialog2(this, parent);
    window->setStyleSheet(QString::fromStdString(style));
    LayoutWidget::createQtWidget(proxy, uiproxy, window);
    window->setWindowTitle(QString::fromStdString(title));
    Qt::WindowFlags flags = Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint;
#ifdef MAC_VREP
    flags |= Qt::Tool;
#else
#ifdef LIN_VREP
    flags |= Qt::Window;
#else
    flags |= Qt::Dialog;
#endif
#endif
    if(resizable) flags |= Qt::WindowMaximizeButtonHint;
    else flags |= Qt::MSWindowsFixedSizeDialogHint;
    if(closeable) flags |= Qt::WindowCloseButtonHint;
    window->setWindowFlags(flags);
    window->setModal(modal);
    //window->setAttribute(Qt::WA_DeleteOnClose);
    if(!activate) window->setAttribute(Qt::WA_ShowWithoutActivating);
    if(qwidget_size.isValid())
        window->resize(qwidget_size);
    window->show();
#ifdef LIN_VREP
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
        window->move(p);
    }
    else if(placement == "absolute")
    {
        window->move(qwidget_pos);
    }
    qwidget = window;
    this->proxy = proxy;
    return window;
}

std::string Window::str()
{
    std::stringstream ss;
    ss << "Window[" << this << "]";
    return ss.str();
}

void Window::hide()
{
    qwidget_pos = qwidget->pos();
    qwidget_size = qwidget->size();
    qwidget_geometry_saved = true;
    visibility_state = false;
    qwidget->hide();
}

void Window::show()
{
    if(qwidget_geometry_saved)
    {
        qwidget->move(qwidget_pos);
        qwidget->resize(qwidget_size);
    }
    visibility_state = true;
    qwidget->show();
}

void Window::onSceneChange(int oldSceneID, int newSceneID)
{
    int mySceneID = proxy->getSceneID();

    DBG << "oldSceneID=" << oldSceneID << ", newSceneID" << newSceneID << std::endl;
    DBG << "mySceneID=" << mySceneID << std::endl;

    if(oldSceneID == mySceneID && newSceneID != mySceneID)
    {
        DBG << "hidden" << std::endl;

        bool saved_visibility_state = visibility_state;
        hide();
        visibility_state = saved_visibility_state;
    }

    if(oldSceneID != mySceneID && newSceneID == mySceneID)
    {
        DBG << "shown" << std::endl;

        if(visibility_state)
            show();
    }
}

