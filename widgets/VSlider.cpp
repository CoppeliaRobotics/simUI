#include "VSlider.h"

#include "UIProxy.h"

#include <QSlider>

VSlider::VSlider()
    : Slider()
{
}

VSlider::~VSlider()
{
}

Qt::Orientation VSlider::getOrientation()
{
    return Qt::Vertical;
}

const char * VSlider::name()
{
    return "vslider";
}

bool VSlider::parse(tinyxml2::XMLElement *e, std::vector<std::string>& errors)
{
    std::string tag(e->Value());
    if(tag != "vslider")
    {
        errors.push_back("element must be <vslider>");
        return false;
    }

    return Slider::parse(e, errors);
}

