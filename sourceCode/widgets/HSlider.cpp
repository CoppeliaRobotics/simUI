#include "HSlider.h"

#include "UI.h"

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

