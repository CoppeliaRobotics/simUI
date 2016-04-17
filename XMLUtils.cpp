#include "XMLUtils.h"

#include <sstream>
#include <cstring>
#include <stdexcept>

bool xmlutils::getAttrBool(tinyxml2::XMLElement *e, const char *name, bool defaultValue)
{
    const char *value = e->Attribute(name);

    if(!value)
        return defaultValue;

    if(strcasecmp(value, "true") == 0)
        return true;
    if(strcasecmp(value, "false") == 0)
        return false;

    std::stringstream ss;
    ss << "invalid value '" << value << "' for attribute " << name << ": must be true or false";
    throw std::range_error(ss.str());
}

int xmlutils::getAttrInt(tinyxml2::XMLElement *e, const char *name, int defaultValue)
{
    const char *value = e->Attribute(name);
    int ret;

    if(!value)
        return defaultValue;

    if(e->QueryIntAttribute(name, &ret) != tinyxml2::XML_NO_ERROR)
    {
        std::stringstream ss;
        ss << "invalid value '" << value << "' for attribute " << name << ": must be integer";
        throw std::range_error(ss.str());
    }

    return ret;
}

std::string xmlutils::getAttrStr(tinyxml2::XMLElement *e, const char *name, std::string defaultValue)
{
    const char *value = e->Attribute(name);

    if(!value)
        return defaultValue;

    return std::string(value);
}

