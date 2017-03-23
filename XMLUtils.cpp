#include "XMLUtils.h"

#include <sstream>
#include <cstring>
#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

bool xmlutils::hasAttr(tinyxml2::XMLElement *e, const char *name)
{
    const char *value = e->Attribute(name);
    if(value) return true;
    else return false;
}

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

std::string xmlutils::getAttrStr(tinyxml2::XMLElement *e, const char *name)
{
    const char *value = e->Attribute(name);

    if(!value)
    {
        std::stringstream ss;
        ss << "missing value for attribute '" << name << "'";
        throw std::range_error(ss.str());
    }

    return std::string(value);
}

std::string xmlutils::getAttrStr(tinyxml2::XMLElement *e, const char *name, std::string defaultValue)
{
    const char *value = e->Attribute(name);

    if(!value)
        return defaultValue;

    return std::string(value);
}

void xmlutils::string2vector(std::string s, std::vector<std::string>& v, int minLength, int maxLength, const char *sep)
{
    boost::char_separator<char> bsep(sep);
    boost::tokenizer< boost::char_separator<char> > tokenizer(s, bsep);
    v.clear();
    for(boost::tokenizer< boost::char_separator<char> >::iterator it = tokenizer.begin(); it != tokenizer.end(); ++it)
    {
        std::string tok = *it;
        v.push_back(tok);
    }
    if(minLength != -1 && maxLength != -1 && (v.size() < minLength || v.size() > maxLength))
    {
        std::stringstream ss;
        ss << "must have ";
        if(minLength != maxLength) ss << "from " << minLength << " to " << maxLength;
        else ss << "exactly " << minLength;
        ss << " elements";
        throw std::range_error(ss.str());
    }
    else if(minLength != -1 && v.size() < minLength)
    {
        std::stringstream ss;
        ss << "must have at least " << minLength << " elements";
        throw std::range_error(ss.str());
    }
    else if(maxLength != -1 && v.size() > maxLength)
    {
        std::stringstream ss;
        ss << "must have at most " << maxLength << " elements";
        throw std::range_error(ss.str());
    }
}

void xmlutils::string2vector(std::string s, std::vector<bool>& v, int minLength, int maxLength, const char *sep)
{
    std::vector<std::string> vs;
    string2vector(s, vs, minLength, maxLength, sep);
    v.clear();
    for(std::vector<std::string>::iterator it = vs.begin(); it != vs.end(); ++it)
        v.push_back(boost::lexical_cast<bool>(*it));
}

void xmlutils::string2vector(std::string s, std::vector<float>& v, int minLength, int maxLength, const char *sep)
{
    std::vector<std::string> vs;
    string2vector(s, vs, minLength, maxLength, sep);
    v.clear();
    for(std::vector<std::string>::iterator it = vs.begin(); it != vs.end(); ++it)
        v.push_back(boost::lexical_cast<float>(*it));
}

void xmlutils::string2vector(std::string s, std::vector<double>& v, int minLength, int maxLength, const char *sep)
{
    std::vector<std::string> vs;
    string2vector(s, vs, minLength, maxLength, sep);
    v.clear();
    for(std::vector<std::string>::iterator it = vs.begin(); it != vs.end(); ++it)
        v.push_back(boost::lexical_cast<double>(*it));
}

void xmlutils::string2vector(std::string s, std::vector<int>& v, int minLength, int maxLength, const char *sep)
{
    std::vector<std::string> vs;
    string2vector(s, vs, minLength, maxLength, sep);
    v.clear();
    for(std::vector<std::string>::iterator it = vs.begin(); it != vs.end(); ++it)
        v.push_back(boost::lexical_cast<int>(*it));
}

std::vector<std::string> xmlutils::getAttrStrV(tinyxml2::XMLElement *e, const char *name, std::string defaultValue, int minLength, int maxLength, const char *sep)
{
    std::vector<std::string> defaultValueV;
    try
    {
        string2vector(defaultValue, defaultValueV, minLength, maxLength, sep);
    }
    catch(std::range_error &ex)
    {
        std::stringstream ss;
        ss << "invalid default value for attribute '" << name << "': " << ex.what();
        throw std::range_error(ss.str());
    }

    return getAttrStrV(e, name, defaultValueV, minLength, maxLength, sep);
}

std::vector<std::string> xmlutils::getAttrStrV(tinyxml2::XMLElement *e, const char *name, std::vector<std::string> defaultValue, int minLength, int maxLength, const char *sep)
{
    if(hasAttr(e, name))
    {
        try
        {
            std::vector<std::string> ret;
            string2vector(getAttrStr(e, name), ret, minLength, maxLength, sep);
            return ret;
        }
        catch(std::range_error &ex)
        {
            std::stringstream ss;
            ss << "attribute '" << name << "' " << ex.what();
            throw std::range_error(ss.str());
        }
    }
    else
    {
        return defaultValue;
    }
}

std::vector<bool> xmlutils::getAttrBoolV(tinyxml2::XMLElement *e, const char *name, std::string defaultValue, int minLength, int maxLength, const char *sep)
{
    std::vector<bool> defaultValueV;
    try
    {
        string2vector(defaultValue, defaultValueV, minLength, maxLength, sep);
    }
    catch(std::range_error &ex)
    {
        std::stringstream ss;
        ss << "invalid default value for attribute '" << name << "': " << ex.what();
        throw std::range_error(ss.str());
    }

    return getAttrBoolV(e, name, defaultValueV, minLength, maxLength, sep);
}

std::vector<bool> xmlutils::getAttrBoolV(tinyxml2::XMLElement *e, const char *name, std::vector<bool> defaultValue, int minLength, int maxLength, const char *sep)
{
    if(hasAttr(e, name))
    {
        try
        {
            std::vector<bool> ret;
            string2vector(getAttrStr(e, name), ret, minLength, maxLength, sep);
            return ret;
        }
        catch(std::range_error &ex)
        {
            std::stringstream ss;
            ss << "attribute '" << name << "' " << ex.what();
            throw std::range_error(ss.str());
        }
    }
    else
    {
        return defaultValue;
    }
}

std::vector<float> xmlutils::getAttrFloatV(tinyxml2::XMLElement *e, const char *name, std::string defaultValue, int minLength, int maxLength, const char *sep)
{
    std::vector<float> defaultValueV;
    try
    {
        string2vector(defaultValue, defaultValueV, minLength, maxLength, sep);
    }
    catch(std::range_error &ex)
    {
        std::stringstream ss;
        ss << "invalid default value for attribute '" << name << "': " << ex.what();
        throw std::range_error(ss.str());
    }

    return getAttrFloatV(e, name, defaultValueV, minLength, maxLength, sep);
}

std::vector<float> xmlutils::getAttrFloatV(tinyxml2::XMLElement *e, const char *name, std::vector<float> defaultValue, int minLength, int maxLength, const char *sep)
{
    if(hasAttr(e, name))
    {
        try
        {
            std::vector<float> ret;
            string2vector(getAttrStr(e, name), ret, minLength, maxLength, sep);
            return ret;
        }
        catch(std::range_error &ex)
        {
            std::stringstream ss;
            ss << "attribute '" << name << "' " << ex.what();
            throw std::range_error(ss.str());
        }
    }
    else
    {
        return defaultValue;
    }
}

std::vector<double> xmlutils::getAttrDoubleV(tinyxml2::XMLElement *e, const char *name, std::string defaultValue, int minLength, int maxLength, const char *sep)
{
    std::vector<double> defaultValueV;
    try
    {
        string2vector(defaultValue, defaultValueV, minLength, maxLength, sep);
    }
    catch(std::range_error &ex)
    {
        std::stringstream ss;
        ss << "invalid default value for attribute '" << name << "': " << ex.what();
        throw std::range_error(ss.str());
    }

    return getAttrDoubleV(e, name, defaultValueV, minLength, maxLength, sep);
}

std::vector<double> xmlutils::getAttrDoubleV(tinyxml2::XMLElement *e, const char *name, std::vector<double> defaultValue, int minLength, int maxLength, const char *sep)
{
    if(hasAttr(e, name))
    {
        try
        {
            std::vector<double> ret;
            string2vector(getAttrStr(e, name), ret, minLength, maxLength, sep);
            return ret;
        }
        catch(std::range_error &ex)
        {
            std::stringstream ss;
            ss << "attribute '" << name << "' " << ex.what();
            throw std::range_error(ss.str());
        }
    }
    else
    {
        return defaultValue;
    }
}

std::vector<int> xmlutils::getAttrIntV(tinyxml2::XMLElement *e, const char *name, std::string defaultValue, int minLength, int maxLength, const char *sep)
{
    std::vector<int> defaultValueV;
    try
    {
        string2vector(defaultValue, defaultValueV, minLength, maxLength, sep);
    }
    catch(std::range_error &ex)
    {
        std::stringstream ss;
        ss << "invalid default value for attribute '" << name << "': " << ex.what();
        throw std::range_error(ss.str());
    }

    return getAttrIntV(e, name, defaultValueV, minLength, maxLength, sep);
}

std::vector<int> xmlutils::getAttrIntV(tinyxml2::XMLElement *e, const char *name, std::vector<int> defaultValue, int minLength, int maxLength, const char *sep)
{
    if(hasAttr(e, name))
    {
        try
        {
            std::vector<int> ret;
            string2vector(getAttrStr(e, name), ret, minLength, maxLength, sep);
            return ret;
        }
        catch(std::range_error &ex)
        {
            std::stringstream ss;
            ss << "attribute '" << name << "' " << ex.what();
            throw std::range_error(ss.str());
        }
    }
    else
    {
        return defaultValue;
    }
}

