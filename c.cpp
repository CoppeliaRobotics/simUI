#include "stubs.h"
#include "v_repPlusPlus/Plugin.h"

VREP_DLLEXPORT int msgBox(int type, int buttons, const char *title, const char *message)
{
    msgBox_in in;
    in.type = type;
    in.buttons = buttons;
    in.title = title;
    in.message = message;

    msgBox_out out;

    msgBox(nullptr, "", &in, &out);

    return out.result;
}

VREP_DLLEXPORT char * fileDialog(int type, const char *title, const char *startPath, const char *initName, const char *extName, const char *ext)
{
    fileDialog_in in;
    in.type = type;
    in.title = title;
    in.startPath = startPath;
    in.initName = initName;
    in.extName = extName;
    in.ext = ext;

    fileDialog_out out;

    fileDialog(nullptr, "", &in, &out);

    int sz = 0;
    for(auto &x : out.result) sz += x.length() + 1;
    simChar *ret = simCreateBuffer(sz), *tmp = ret;
    for(auto &x : out.result)
    {
        strcpy(tmp, x.c_str());
        tmp += x.length();
        *tmp = ';';
        tmp++;
    }
    tmp--;
    *tmp = '\0';
    return ret;
}

