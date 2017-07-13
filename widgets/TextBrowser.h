#ifndef TEXTBROWSER_H_INCLUDED
#define TEXTBROWSER_H_INCLUDED

#include "config.h"

#include <vector>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UIProxy;

#include "Widget.h"
#include "Event.h"

class TextBrowser : public Widget, public EventOnChangeString, public EventOnLinkActivated
{
protected:
    std::string text;
    bool html;
    bool read_only;

public:
    TextBrowser();
    virtual ~TextBrowser();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    void setText(std::string text, bool suppressSignals);
    std::string getText();
    void setUrl(std::string url);
    std::string getUrl();

    friend class UIFunctions;
};

#endif // TEXTBROWSER_H_INCLUDED

