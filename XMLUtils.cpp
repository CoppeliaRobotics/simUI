#include "XMLUtils.h"

#include <sstream>
#include <cstring>
#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

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
    ss << "invalid value '" << value << "' for attribute '" << name << "': must be true or false";
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
        ss << "invalid value '" << value << "' for attribute '" << name << "': must be integer";
        throw std::range_error(ss.str());
    }

    return ret;
}

float xmlutils::getAttrFloat(tinyxml2::XMLElement *e, const char *name, float defaultValue)
{
    const char *value = e->Attribute(name);
    float ret;

    if(!value)
        return defaultValue;

    if(e->QueryFloatAttribute(name, &ret) != tinyxml2::XML_NO_ERROR)
    {
        std::stringstream ss;
        ss << "invalid value '" << value << "' for attribute '" << name << "': must be float";
        throw std::range_error(ss.str());
    }

    return ret;
}

double xmlutils::getAttrDouble(tinyxml2::XMLElement *e, const char *name, double defaultValue)
{
    const char *value = e->Attribute(name);
    double ret;

    if(!value)
        return defaultValue;

    if(e->QueryDoubleAttribute(name, &ret) != tinyxml2::XML_NO_ERROR)
    {
        std::stringstream ss;
        ss << "invalid value '" << value << "' for attribute '" << name << "': must be double";
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

std::vector<std::string> xmlutils::getAttrStrV(tinyxml2::XMLElement *e, const char *name, std::string defaultValue, int minLength, int maxLength, const char *sep)
{
    std::string s = getAttrStr(e, name, defaultValue);

    boost::char_separator<char> bsep(sep);
    boost::tokenizer< boost::char_separator<char> > tokenizer(s, bsep);
    std::vector<std::string> ret;
    for(boost::tokenizer< boost::char_separator<char> >::iterator it = tokenizer.begin(); it != tokenizer.end(); ++it)
    {
        std::string tok = *it;
        ret.push_back(tok);
    }
    if(minLength != -1 && maxLength != -1 && (ret.size() < minLength || ret.size() > maxLength))
    {
        std::stringstream ss;
        ss << "attribute '" << name << "' must have ";
        if(minLength != maxLength) ss << "from " << minLength << " to " << maxLength;
        else ss << "exactly " << minLength;
        ss << " elements";
        throw std::range_error(ss.str());
    }
    else if(minLength != -1 && ret.size() < minLength)
    {
        std::stringstream ss;
        ss << "attribute '" << name << "' must have at least " << minLength << " elements";
        throw std::range_error(ss.str());
    }
    else if(maxLength != -1 && ret.size() > maxLength)
    {
        std::stringstream ss;
        ss << "attribute '" << name << "' must have at most " << maxLength << " elements";
        throw std::range_error(ss.str());
    }
    return ret;
}

std::vector<bool> xmlutils::getAttrBoolV(tinyxml2::XMLElement *e, const char *name, std::string defaultValue, int minLength, int maxLength, const char *sep)
{
    std::vector<std::string> v = getAttrStrV(e, name, defaultValue, minLength, maxLength, sep);
    std::vector<bool> ret;
    for(std::vector<std::string>::iterator it = v.begin(); it != v.end(); ++it)
    {
        ret.push_back(boost::lexical_cast<bool>(*it));
    }
    return ret;
}

std::vector<float> xmlutils::getAttrFloatV(tinyxml2::XMLElement *e, const char *name, std::string defaultValue, int minLength, int maxLength, const char *sep)
{
    std::vector<std::string> v = getAttrStrV(e, name, defaultValue, minLength, maxLength, sep);
    std::vector<float> ret;
    for(std::vector<std::string>::iterator it = v.begin(); it != v.end(); ++it)
    {
        ret.push_back(boost::lexical_cast<float>(*it));
    }
    return ret;
}

std::vector<double> xmlutils::getAttrDoubleV(tinyxml2::XMLElement *e, const char *name, std::string defaultValue, int minLength, int maxLength, const char *sep)
{
    std::vector<std::string> v = getAttrStrV(e, name, defaultValue, minLength, maxLength, sep);
    std::vector<double> ret;
    for(std::vector<std::string>::iterator it = v.begin(); it != v.end(); ++it)
    {
        ret.push_back(boost::lexical_cast<double>(*it));
    }
    return ret;
}

std::vector<int> xmlutils::getAttrIntV(tinyxml2::XMLElement *e, const char *name, std::string defaultValue, int minLength, int maxLength, const char *sep)
{
    std::vector<std::string> v = getAttrStrV(e, name, defaultValue, minLength, maxLength, sep);
    std::vector<int> ret;
    for(std::vector<std::string>::iterator it = v.begin(); it != v.end(); ++it)
    {
        ret.push_back(boost::lexical_cast<int>(*it));
    }
    return ret;
}

