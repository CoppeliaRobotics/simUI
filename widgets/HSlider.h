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

    const char * name();
    
    bool parse(tinyxml2::XMLElement *e, std::vector<std::string>& errors);

    friend class UIFunctions;
};

#endif // HSLIDER_H_INCLUDED

