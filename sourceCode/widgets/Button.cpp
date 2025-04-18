#include "Button.h"

#include "XMLUtils.h"

#include "UI.h"

#include <QPushButton>

Button::Button()
    : Widget("button")
{
}

Button::~Button()
{
}

void Button::parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(parent, widgets, e);

    text = xmlutils::getAttrStr(e, "text", "???");

    defaulT = xmlutils::getAttrBool(e, "default", false);

    auto_repeat = xmlutils::getAttrBool(e, "auto-repeat", false);

    auto_repeat_delay = xmlutils::getAttrInt(e, "auto-repeat-delay", 0);

    auto_repeat_interval = xmlutils::getAttrInt(e, "auto-repeat-interval", 200);

    checked = xmlutils::getAttrBool(e, "checked", false);

    checkable = xmlutils::getAttrBool(e, "checkable", false);

    auto_exclusive = xmlutils::getAttrBool(e, "auto-exclusive", false);

    onclick = xmlutils::getAttrStr(e, "on-click", "");

    icon = xmlutils::getAttrStr(e, "icon", "");

    stretch = xmlutils::getAttrBool(e, "stretch", true);
}

QWidget * Button::createQtWidget(Proxy *proxy, UI *ui, QWidget *parent)
{
    QPushButton *button = new QPushButton(QString::fromStdString(text), parent);
    button->setEnabled(enabled);
    button->setVisible(visible);
    button->setStyleSheet(QString::fromStdString(style));
    button->setAutoDefault(false);
    button->setDefault(defaulT);
    button->setAutoRepeat(auto_repeat);
    button->setAutoRepeatDelay(auto_repeat_delay);
    button->setAutoRepeatInterval(auto_repeat_interval);
    button->setCheckable(checkable);
    button->setAutoExclusive(auto_exclusive);
    button->setChecked(checked);
    button->setProperty("stretch", stretch);
    if(!icon.empty())
    {
        QIcon qicon;
        if(icon.substr(0, 10) == "default://")
        {
            icon = icon.substr(10);
            QStyle::StandardPixmap sp = QStyle::SP_MessageBoxQuestion;
            if(0) {}
#define M(n) else if(icon == #n) sp = QStyle::n
#include "StandardIcons.h"
#undef M
            button->setIcon(button->style()->standardIcon(sp));
        }
    }
    QObject::connect(button, &QPushButton::clicked, ui, &UI::onButtonClick);
    setQWidget(button);
    setProxy(proxy);
    return button;
}

void Button::setText(std::string text)
{
    QPushButton *button = static_cast<QPushButton*>(getQWidget());
    button->setText(QString::fromStdString(text));
}

void Button::setPressed(bool pressed)
{
    QPushButton *button = static_cast<QPushButton*>(getQWidget());
    button->setChecked(pressed);
}

