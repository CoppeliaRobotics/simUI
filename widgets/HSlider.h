#ifndef HSLIDER_H_INCLUDED
#define HSLIDER_H_INCLUDED

#include "Slider.h"

class HSlider : public Slider
{
protected:
    Qt::Orientation getOrientation();

public:
    HSlider();
    virtual ~HSlider();

    friend class UIFunctions;
};

#endif // HSLIDER_H_INCLUDED

