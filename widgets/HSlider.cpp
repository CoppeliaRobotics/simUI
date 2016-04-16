#include "HSlider.h"

#include "UIProxy.h"

#include <QSlider>

HSlider::HSlider()
    : Slider()
{
}

HSlider::~HSlider()
{
}

Qt::Orientation HSlider::getOrientation()
{
    return Qt::Horizontal;
}

const char * HSlider::name()
{
    return "hslider";
}

bool HSlider::parse(tinyxml2::XMLElement *e, std::vector<std::string>& errors)
{
    std::string tag(e->Value());
    if(tag != "hslider")
    {
        errors.push_back("element must be <hslider>");
        return false;
    }

    return Slider::parse(e, errors);
}

