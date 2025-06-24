#include "XMLUtils.h"

#include <sstream>
#include <cstring>
#include <stdexcept>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <set>
#include <vector>

#include <simPlusPlus-2/Lib.h>
#include "stubs.h"

static std::set<std::string> repDepNames;

#define REPORT_DEPRECATED_ATTRIB_NAME(dname, name)      \
    if(repDepNames.find(dname) == repDepNames.end())    \
    {                                                   \
        std::stringstream ss;                           \
        ss << "attribute name '" << dname               \
           << "' is deprecated. please use '" << name   \
           << "' instead.";                             \
        sim::addLog(sim_verbosity_warnings, ss.str().c_str());  \
        repDepNames.insert(dname);                      \
    }

#define CHECK_FOR_DEPRECATED_ATTRIB_NAME(name, value)   \
    if(!value && containsHyphens(name))                 \
    {                                                   \
        std::string dname = stripHyphens(name);         \
        value = e->Attribute(dname.c_str());            \
        if(value)                                       \
        {                                               \
            REPORT_DEPRECATED_ATTRIB_NAME(dname, name); \
            markKnownAttribute(dname);                  \
        }                                               \
    }

bool xmlutils::containsHyphens(const std::string &name)
{
    return name.find('-') != std::string::npos;
}

std::string xmlutils::stripHyphens(const std::string &name)
{
    std::string ret = "";
    for(size_t i = 0; i < name.length(); i++)
        if(name[i] != '-')
            ret += name[i];
    return ret;
}

bool xmlutils::hasAttr(tinyxml2::XMLElement *e, std::string name)
{
    markKnownAttribute(name);

    const char *value = e->Attribute(name.c_str());

    CHECK_FOR_DEPRECATED_ATTRIB_NAME(name, value);

    if(value) return true;
    else return false;
}

bool xmlutils::getAttrBool(tinyxml2::XMLElement *e, std::string name, bool defaultValue)
{
    if(!hasAttr(e, name)) return defaultValue;

    std::string value = getAttrStr(e, name);

    if(value == "true")
        return true;
    if(value == "false")
        return false;

    std::stringstream ss;
    ss << "invalid value '" << value << "' for attribute '" << name << "': must be true or false";
    throw std::range_error(ss.str());
}

int xmlutils::getAttrInt(tinyxml2::XMLElement *e, std::string name, int defaultValue)
{
    if(!hasAttr(e, name)) return defaultValue;

    std::string value = getAttrStr(e, name);

    return boost::lexical_cast<int>(value);
}

float xmlutils::getAttrFloat(tinyxml2::XMLElement *e, std::string name, float defaultValue)
{
    if(!hasAttr(e, name)) return defaultValue;

    std::string value = getAttrStr(e, name);

    return boost::lexical_cast<float>(value);
}

double xmlutils::getAttrDouble(tinyxml2::XMLElement *e, std::string name, double defaultValue)
{
    if(!hasAttr(e, name)) return defaultValue;

    std::string value = getAttrStr(e, name);

    return boost::lexical_cast<double>(value);
}

std::string xmlutils::getAttrStr(tinyxml2::XMLElement *e, std::string name)
{
    markKnownAttribute(name);

    const char *value = e->Attribute(name.c_str());

    CHECK_FOR_DEPRECATED_ATTRIB_NAME(name, value);

    if(!value)
    {
        std::stringstream ss;
        ss << "missing value for attribute '" << name << "'";
        throw std::range_error(ss.str());
    }

    return std::string(value);
}

std::string xmlutils::getAttrStr(tinyxml2::XMLElement *e, std::string name, std::string defaultValue)
{
    if(!hasAttr(e, name)) return defaultValue;

    std::string value = getAttrStr(e, name);

    return value;
}

std::string xmlutils::getAttrStrEnum(tinyxml2::XMLElement *e, std::string name, std::string defaultValue, const std::vector<std::string> &validValues)
{
    std::string value = getAttrStr(e, name, defaultValue);

    if(std::find(validValues.begin(), validValues.end(), value) == validValues.end()) {
        std::ostringstream oss;
        oss << "invalid value for attribute '" << name << "': '" << value << "' (must be one of: ";
        for(size_t i = 0; i < validValues.size(); ++i)
            oss << (i > 0 ? ", " : "") << "'" << validValues[i] << "'";
        oss << ")";
        throw std::runtime_error(oss.str());
    }

    return value;
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

std::vector<std::string> xmlutils::getAttrStrV(tinyxml2::XMLElement *e, std::string name, std::string defaultValue, int minLength, int maxLength, const char *sep)
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

std::vector<std::string> xmlutils::getAttrStrV(tinyxml2::XMLElement *e, std::string name, std::vector<std::string> defaultValue, int minLength, int maxLength, const char *sep)
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

std::vector<bool> xmlutils::getAttrBoolV(tinyxml2::XMLElement *e, std::string name, std::string defaultValue, int minLength, int maxLength, const char *sep)
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

std::vector<bool> xmlutils::getAttrBoolV(tinyxml2::XMLElement *e, std::string name, std::vector<bool> defaultValue, int minLength, int maxLength, const char *sep)
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

std::vector<float> xmlutils::getAttrFloatV(tinyxml2::XMLElement *e, std::string name, std::string defaultValue, int minLength, int maxLength, const char *sep)
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

std::vector<float> xmlutils::getAttrFloatV(tinyxml2::XMLElement *e, std::string name, std::vector<float> defaultValue, int minLength, int maxLength, const char *sep)
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

std::vector<double> xmlutils::getAttrDoubleV(tinyxml2::XMLElement *e, std::string name, std::string defaultValue, int minLength, int maxLength, const char *sep)
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

std::vector<double> xmlutils::getAttrDoubleV(tinyxml2::XMLElement *e, std::string name, std::vector<double> defaultValue, int minLength, int maxLength, const char *sep)
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

std::vector<int> xmlutils::getAttrIntV(tinyxml2::XMLElement *e, std::string name, std::string defaultValue, int minLength, int maxLength, const char *sep)
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

std::vector<int> xmlutils::getAttrIntV(tinyxml2::XMLElement *e, std::string name, std::vector<int> defaultValue, int minLength, int maxLength, const char *sep)
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

namespace xmlutils { static std::vector<std::set<std::string> > knownAttributes; };

void xmlutils::resetKnownAttributes()
{
    knownAttributes.push_back(std::set<std::string>());
}

void xmlutils::markKnownAttribute(std::string a)
{
    if(knownAttributes.size())
        knownAttributes.back().insert(a);
}

#include <iostream>

std::set<std::string> xmlutils::getUnknownAttributes(tinyxml2::XMLElement *e)
{
    if(knownAttributes.size() == 0)
        return std::set<std::string>();
    std::set<std::string> &knownAttributes1 = knownAttributes.back();
    std::set<std::string> ret;
    for(const tinyxml2::XMLAttribute *a = e->FirstAttribute(); a; a = a->Next())
    {
        std::string attr(a->Name());
        if(knownAttributes1.find(attr) == knownAttributes1.end())
            ret.insert(attr);
    }
    knownAttributes.pop_back();
    return ret;
}

void xmlutils::reportUnknownAttributes(const std::string &widget, tinyxml2::XMLElement *e)
{
    BOOST_FOREACH(const std::string &a, getUnknownAttributes(e))
    {
        sim::addLog(sim_verbosity_warnings, "unknown UI XML attribute '%s' in widget '%s'", a, widget);
    }
}

std::string xmlutils::elementToString(tinyxml2::XMLElement *element)
{
    tinyxml2::XMLPrinter printer;
    element->Accept(&printer);
    std::stringstream ss;
    ss << printer.CStr();
    return ss.str();
}

