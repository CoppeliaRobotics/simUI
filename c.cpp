#include "stubs.h"
#include "simPlusPlus/Plugin.h"

SIM_DLLEXPORT int customUi_msgBox(int type, int buttons, const char *title, const char *message)
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

SIM_DLLEXPORT char * customUi_fileDialog(int type, const char *title, const char *startPath, const char *initName, const char *extName, const char *ext, int native)
{
    fileDialog_in in;
    in.type = type;
    in.title = title;
    in.startPath = startPath;
    in.initName = initName;
    in.extName = extName;
    in.ext = ext;
    in.native = !!native;

    fileDialog_out out;

    fileDialog(nullptr, "", &in, &out);

    if(out.result.empty()) return nullptr;

    int sz = 0;
    for(auto &x : out.result) sz += x.length() + 1;
    char *ret = simCreateBuffer(sz), *tmp = ret;
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

SIM_DLLEXPORT float * customUi_colorDialog(const float *initColor, const char *title, int showAlphaChannel, int native)
{
    colorDialog_in in;
    for(int i = 0; i < (showAlphaChannel ? 4 : 3); i++)
        in.initColor.push_back(initColor[i]);
    in.title = title;
    in.showAlphaChannel = showAlphaChannel;
    in.native = !!native;

    colorDialog_out out;

    colorDialog(nullptr, "", &in, &out);

    if(!out.result || out.result->empty()) return nullptr;

    float *ret = (float*)simCreateBuffer(sizeof(float) * (showAlphaChannel ? 4 : 3));
    ret[0] = out.result->at(0);
    ret[1] = out.result->at(1);
    ret[2] = out.result->at(2);
    if(showAlphaChannel) ret[3] = out.result->at(3);
    return ret;
}

SIM_DLLEXPORT char * customUi_inputDialog(const char *initValue, const char *label, const char *title)
{
    inputDialog_in in;
    if(initValue)
        in.initValue = initValue;
    if(label)
        in.label = label;
    if(title)
        in.title = title;

    inputDialog_out out;

    inputDialog(nullptr, "", &in, &out);

    if(!out.result) return nullptr;

    char *ret = simCreateBuffer(out.result->length());
    std::memcpy(ret, out.result->data(), out.result->length());
    return ret;
}

