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

    friend class SIM;
};

#endif // VSLIDER_H_INCLUDED

