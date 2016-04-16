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

