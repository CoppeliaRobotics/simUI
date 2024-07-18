#include "Label.h"

#include "XMLUtils.h"

#include "UI.h"

#include <QLabel>

Label::Label()
    : Widget("label")
{
}

Label::~Label()
{
}

void Label::parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(parent, widgets, e);

    text = xmlutils::getAttrStr(e, "text", "");

    if(xmlutils::hasAttr(e, "wordwrap"))
        sim::addLog(sim_verbosity_warnings, "attribute name 'wordwrap' is deprecated. please use 'word-wrap' instead.");
    wordWrap = xmlutils::getAttrBool(e, "word-wrap", xmlutils::getAttrBool(e, "wordwrap", false));

    onLinkActivated = xmlutils::getAttrStr(e, "on-link-activated", "");
}

QWidget * Label::createQtWidget(Proxy *proxy, UI *ui, QWidget *parent)
{
    QLabel *label = new QLabel(QString::fromStdString(text), parent);
    label->setEnabled(enabled);
    label->setVisible(visible);
    label->setStyleSheet(QString::fromStdString(style));
    label->setWordWrap(wordWrap);
    label->setOpenExternalLinks(false);
    QObject::connect(label, &QLabel::linkActivated, ui, &UI::onLinkActivated);
    setQWidget(label);
    setProxy(proxy);
    return label;
}

void Label::setText(std::string text, bool suppressSignals)
{
    QLabel *qlabel = static_cast<QLabel*>(getQWidget());
    bool oldSignalsState = qlabel->blockSignals(suppressSignals);
    qlabel->setText(QString::fromStdString(text));
    qlabel->blockSignals(oldSignalsState);
}

std::string Label::getText()
{
    QLabel *qlabel = static_cast<QLabel*>(getQWidget());
    return qlabel->text().toStdString();
}

