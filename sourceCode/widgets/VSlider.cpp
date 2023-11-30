#include "VSlider.h"

#include "UI.h"

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

