#include "HSlider.h"

#include "UIProxy.h"

#include <QSlider>

HSlider::HSlider()
    : Slider("hslider")
{
}

HSlider::~HSlider()
{
}

Qt::Orientation HSlider::getOrientation()
{
    return Qt::Horizontal;
}

