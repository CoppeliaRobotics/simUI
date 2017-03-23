#ifndef XMLUTILS_H_INCLUDED
#define XMLUTILS_H_INCLUDED

#include <string>
#include <vector>

#include "tinyxml2.h"

namespace xmlutils
{
    bool hasAttr(tinyxml2::XMLElement *e, const char *name);

    bool getAttrBool(tinyxml2::XMLElement *e, const char *name, bool defaultValue);

    int getAttrInt(tinyxml2::XMLElement *e, const char *name, int defaultValue);

    float getAttrFloat(tinyxml2::XMLElement *e, const char *name, float defaultValue);

    double getAttrDouble(tinyxml2::XMLElement *e, const char *name, double defaultValue);

    std::string getAttrStr(tinyxml2::XMLElement *e, const char *name);

    std::string getAttrStr(tinyxml2::XMLElement *e, const char *name, std::string defaultValue);

    void string2vector(std::string s, std::vector<std::string>& v, int minLength, int maxLength, const char *sep);

    void string2vector(std::string s, std::vector<bool>& v, int minLength, int maxLength, const char *sep);

    void string2vector(std::string s, std::vector<float>& v, int minLength, int maxLength, const char *sep);

    void string2vector(std::string s, std::vector<double>& v, int minLength, int maxLength, const char *sep);

    void string2vector(std::string s, std::vector<int>& v, int minLength, int maxLength, const char *sep);

    std::vector<std::string> getAttrStrV(tinyxml2::XMLElement *e, const char *name, std::string defaultValue, int minLength, int maxLength, const char *sep);

    std::vector<std::string> getAttrStrV(tinyxml2::XMLElement *e, const char *name, std::vector<std::string> defaultValue, int minLength, int maxLength, const char *sep);

    std::vector<bool> getAttrBoolV(tinyxml2::XMLElement *e, const char *name, std::string defaultValue, int minLength, int maxLength, const char *sep);

    std::vector<bool> getAttrBoolV(tinyxml2::XMLElement *e, const char *name, std::vector<bool> defaultValue, int minLength, int maxLength, const char *sep);

    std::vector<float> getAttrFloatV(tinyxml2::XMLElement *e, const char *name, std::string defaultValue, int minLength, int maxLength, const char *sep);

    std::vector<float> getAttrFloatV(tinyxml2::XMLElement *e, const char *name, std::vector<float> defaultValue, int minLength, int maxLength, const char *sep);

    std::vector<double> getAttrDoubleV(tinyxml2::XMLElement *e, const char *name, std::string defaultValue, int minLength, int maxLength, const char *sep);

    std::vector<double> getAttrDoubleV(tinyxml2::XMLElement *e, const char *name, std::vector<double> defaultValue, int minLength, int maxLength, const char *sep);

    std::vector<int> getAttrIntV(tinyxml2::XMLElement *e, const char *name, std::string defaultValue, int minLength, int maxLength, const char *sep);

    std::vector<int> getAttrIntV(tinyxml2::XMLElement *e, const char *name, std::vector<int> defaultValue, int minLength, int maxLength, const char *sep);
};

#endif // XMLUTILS_H_INCLUDED

