#ifndef GROUP_H_INCLUDED
#define GROUP_H_INCLUDED

#include <vector>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UIProxy;

#include "Widget.h"
#include "LayoutWidget.h"

class Group : public Widget, public LayoutWidget
{
protected:
    std::string text;

public:
    Group();
    virtual ~Group();

    const char * name();
    
    bool parse(tinyxml2::XMLElement *e, std::vector<std::string>& errors);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class UIFunctions;
};

#endif // GROUP_H_INCLUDED

