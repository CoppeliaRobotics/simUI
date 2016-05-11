#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED

#include <vector>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UIProxy;

#include "Widget.h"

class Image : public Widget
{
protected:
    int width;
    int height;
    std::string file;

public:
    Image();
    virtual ~Image();
    
    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    friend class UIFunctions;
};

#endif // IMAGE_H_INCLUDED

