#ifndef VSLIDER_H_INCLUDED
#define VSLIDER_H_INCLUDED

#include "config.h"

#include "Slider.h"

class VSlider : public Slider
{
protected:
    Qt::Orientation getOrientation();

public:
    VSlider();
    virtual ~VSlider();

    friend class UIFunctions;
};

#endif // VSLIDER_H_INCLUDED

