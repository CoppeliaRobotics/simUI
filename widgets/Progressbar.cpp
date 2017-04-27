#include "Progressbar.h"

#include "XMLUtils.h"

#include "UIProxy.h"

#include <QProgressBar>

Progressbar::Progressbar()
    : Widget("progressbar")
{
}

Progressbar::~Progressbar()
{
}

void Progressbar::parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(parent, widgets, e);

    minimum = xmlutils::getAttrInt(e, "minimum", 0);

    maximum = xmlutils::getAttrInt(e, "maximum", 100);

    std::string orientation_str = xmlutils::getAttrStr(e, "orientation", "horizontal");
    if(orientation_str == "horizontal") orientation = Qt::Horizontal;
    else if(orientation_str == "vertical") orientation = Qt::Vertical;
    else throw std::range_error("invalid valud for orientation attribute");

    inverted = xmlutils::getAttrBool(e, "inverted", false);

    value = xmlutils::getAttrInt(e, "value", 0);

    text_visible = xmlutils::getAttrBool(e, "text-visible", true);
}

QWidget * Progressbar::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QProgressBar *progressbar = new QProgressBar(parent);
    progressbar->setEnabled(enabled);
    progressbar->setVisible(visible);
    progressbar->setStyleSheet(QString::fromStdString(style));
    progressbar->setMinimum(minimum);
    progressbar->setMaximum(maximum);
    progressbar->setOrientation(orientation);
    progressbar->setInvertedAppearance(inverted);
    progressbar->setValue(value);
    progressbar->setTextVisible(text_visible);
    setQWidget(progressbar);
    setProxy(proxy);
    return progressbar;
}

void Progressbar::setValue(int value)
{
    QProgressBar *progressbar = static_cast<QProgressBar*>(getQWidget());
    progressbar->setValue(value);
}

