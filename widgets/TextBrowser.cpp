#include "TextBrowser.h"

#include "XMLUtils.h"

#include "UI.h"

#include <QTextBrowser>

TextBrowser::TextBrowser()
    : Widget("text-browser")
{
}

TextBrowser::~TextBrowser()
{
}

void TextBrowser::parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(parent, widgets, e);

    text = xmlutils::getAttrStr(e, "text", "");

    if(xmlutils::hasAttr(e, "html") && !xmlutils::hasAttr(e, "type"))
    {
        sim::addLog(sim_verbosity_warnings, "attribute name 'html' is deprecated. please use 'type' instead.");
        bool html = xmlutils::getAttrBool(e, "html", true);
        type = html ? "html" : "plain";
    }
    else
    {
        type = xmlutils::getAttrStr(e, "type", "html");
        if(type != "plain" && type != "html" && type != "markdown")
            throw std::runtime_error("unsupported type: " + type);
    }

    read_only = xmlutils::getAttrBool(e, "read-only", true);

    onchange = xmlutils::getAttrStr(e, "on-change", "");

    onLinkActivated = xmlutils::getAttrStr(e, "on-link-activated", "");
}

QWidget * TextBrowser::createQtWidget(Proxy *proxy, UI *ui, QWidget *parent)
{
    QTextBrowser *qtextbrowser = new QTextBrowser(parent);
    qtextbrowser->setEnabled(enabled);
    qtextbrowser->setVisible(visible);
    qtextbrowser->setStyleSheet(QString::fromStdString(style));
    if(type == "html")
        qtextbrowser->setHtml(QString::fromStdString(text));
    else if(type == "markdown")
        qtextbrowser->setMarkdown(QString::fromStdString(text));
    else
        qtextbrowser->setPlainText(QString::fromStdString(text));
    qtextbrowser->setReadOnly(read_only);
    qtextbrowser->setOpenLinks(false);
    QObject::connect(qtextbrowser, &QTextBrowser::textChanged, ui, &UI::onTextChanged);
    QObject::connect(qtextbrowser, &QTextBrowser::anchorClicked, ui, &UI::onAnchorClicked);
    setQWidget(qtextbrowser);
    setProxy(proxy);
    return qtextbrowser;
}

void TextBrowser::setText(std::string text, bool suppressSignals)
{
    QTextBrowser *qtextbrowser = static_cast<QTextBrowser*>(getQWidget());
    bool oldSignalsState = qtextbrowser->blockSignals(suppressSignals);
    if(type == "html")
        qtextbrowser->setHtml(QString::fromStdString(text));
    else if(type == "markdown")
        qtextbrowser->setMarkdown(QString::fromStdString(text));
    else
        qtextbrowser->setPlainText(QString::fromStdString(text));
    qtextbrowser->blockSignals(oldSignalsState);
}

std::string TextBrowser::getText()
{
    QTextBrowser *qtextbrowser = static_cast<QTextBrowser*>(getQWidget());
    if(type == "html")
        return qtextbrowser->toHtml().toStdString();
    else
        return qtextbrowser->toPlainText().toStdString();
}

void TextBrowser::setUrl(std::string url)
{
    QTextBrowser *qtextbrowser = static_cast<QTextBrowser*>(getQWidget());
    qtextbrowser->setSource(QUrl(QString::fromStdString(url)));
}

std::string TextBrowser::getUrl()
{
    QTextBrowser *qtextbrowser = static_cast<QTextBrowser*>(getQWidget());
    return qtextbrowser->source().url().toStdString();
}

