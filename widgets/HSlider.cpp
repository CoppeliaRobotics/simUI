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

