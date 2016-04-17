#ifndef XMLUTILS_H_INCLUDED
#define XMLUTILS_H_INCLUDED

#include <string>

#include "tinyxml2.h"

namespace xmlutils
{
    bool getAttrBool(tinyxml2::XMLElement *e, const char *name, bool defaultValue);

    int getAttrInt(tinyxml2::XMLElement *e, const char *name, int defaultValue);

    std::string getAttrStr(tinyxml2::XMLElement *e, const char *name, std::string defaultValue);
};

#endif // XMLUTILS_H_INCLUDED

