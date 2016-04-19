#include "VSlider.h"

#include "UIProxy.h"

#include <QSlider>

VSlider::VSlider()
    : Slider("vslider")
{
}

VSlider::~VSlider()
{
}

Qt::Orientation VSlider::getOrientation()
{
    return Qt::Vertical;
}

