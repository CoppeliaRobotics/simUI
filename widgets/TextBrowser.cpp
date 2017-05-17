#include "TextBrowser.h"

#include "XMLUtils.h"

#include "UIProxy.h"

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

    html = xmlutils::getAttrBool(e, "html", true);

    read_only = xmlutils::getAttrBool(e, "read-only", true);

    onchange = xmlutils::getAttrStr(e, "on-change", "");

    onLinkActivated = xmlutils::getAttrStr(e, "on-link-activated", "");
}

QWidget * TextBrowser::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QTextBrowser *qtextbrowser = new QTextBrowser(parent);
    qtextbrowser->setEnabled(enabled);
    qtextbrowser->setVisible(visible);
    qtextbrowser->setStyleSheet(QString::fromStdString(style));
    if(html)
        qtextbrowser->setHtml(QString::fromStdString(text));
    else
        qtextbrowser->setPlainText(QString::fromStdString(text));
    qtextbrowser->setReadOnly(read_only);
    qtextbrowser->setOpenLinks(false);
    QObject::connect(qtextbrowser, &QTextBrowser::textChanged, uiproxy, &UIProxy::onTextChanged);
    QObject::connect(qtextbrowser, &QTextBrowser::anchorClicked, uiproxy, &UIProxy::onAnchorClicked);
    setQWidget(qtextbrowser);
    setProxy(proxy);
    return qtextbrowser;
}

void TextBrowser::setText(std::string text, bool suppressSignals)
{
    QTextBrowser *qtextbrowser = static_cast<QTextBrowser*>(getQWidget());
    bool oldSignalsState = qtextbrowser->blockSignals(suppressSignals);
    if(html)
        qtextbrowser->setHtml(QString::fromStdString(text));
    else
        qtextbrowser->setPlainText(QString::fromStdString(text));
    qtextbrowser->blockSignals(oldSignalsState);
}

std::string TextBrowser::getText()
{
    QTextBrowser *qtextbrowser = static_cast<QTextBrowser*>(getQWidget());
    if(html)
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

