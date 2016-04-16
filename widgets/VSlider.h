#ifndef VSLIDER_H_INCLUDED
#define VSLIDER_H_INCLUDED

#include "Slider.h"

class VSlider : public Slider
{
protected:
    Qt::Orientation getOrientation();

public:
    VSlider();
    virtual ~VSlider();

    const char * name();
    
    bool parse(tinyxml2::XMLElement *e, std::vector<std::string>& errors);

    friend class UIFunctions;
};

#endif // VSLIDER_H_INCLUDED

