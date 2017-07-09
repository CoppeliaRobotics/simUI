#ifndef SCENE3D_H_INCLUDED
#define SCENE3D_H_INCLUDED

#include <vector>
#include <map>
#include <string>

#include <QWidget>

#include <QtCore/QObject>

#include <Qt3DCore/qentity.h>
#include <Qt3DCore/qtransform.h>

#include <Qt3DRender/qcamera.h>
#include <Qt3DCore/qentity.h>
#include <Qt3DRender/qcameralens.h>

#include <Qt3DInput/QInputAspect>

#include <Qt3DExtras/qtorusmesh.h>
#include <Qt3DRender/qmesh.h>
#include <Qt3DRender/qtechnique.h>
#include <Qt3DRender/qmaterial.h>
#include <Qt3DRender/qeffect.h>
#include <Qt3DRender/qtexture.h>
#include <Qt3DRender/qrenderpass.h>
#include <Qt3DRender/qsceneloader.h>
#include <Qt3DRender/qpointlight.h>

#include <Qt3DCore/qtransform.h>
#include <Qt3DCore/qaspectengine.h>

#include <Qt3DRender/qrenderaspect.h>
#include <Qt3DExtras/qforwardrenderer.h>

#include <Qt3DExtras/qt3dwindow.h>
#include <Qt3DExtras/qfirstpersoncameracontroller.h>

#include <Qt3DExtras/QTorusMesh>
#include <Qt3DExtras/QConeMesh>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QPlaneMesh>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QPhongMaterial>

#include "tinyxml2.h"

#include "stubs.h"

class Proxy;
class UIProxy;

#include "Widget.h"
#include "Event.h"

struct Node;

class Scene3D : public Widget
{
protected:
    std::map<int, Qt3DCore::QNode*> nodeById_;

    std::vector<int> clearColor;
    std::vector<Node> nodes;

    Qt3DExtras::Qt3DWindow *view;
    Qt3DCore::QEntity *rootEntity;

    void createNodes(Qt3DCore::QNode *parentQNode, int parentId, std::vector<Node> &nodes);
    Qt3DCore::QNode * createNode(Qt3DCore::QNode *parentQNode, int parentId, Node &node);

public:
    Scene3D();
    virtual ~Scene3D();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    Qt3DCore::QNode * nodeById(int id);

    Qt3DCore::QNode * addNode(int id, int parentId, int type);
    void removeNode(int id);
    void enableNode(int id, bool enabled);
    void setIntParameter(int id, std::string param, int value);
    void setFloatParameter(int id, std::string param, float value);
    void setStringParameter(int id, std::string param, std::string value);
    void setVector3Parameter(int id, std::string param, float x, float y, float z);
    void setVector4Parameter(int id, std::string param, float x, float y, float z, float w);

    friend class UIFunctions;
};

struct NodeParam {
    std::string name;
    bool isInt;
    int intValue;
    bool isFloat;
    float floatValue;
    bool isString;
    std::string stringValue;
    bool isVector3;
    bool isVector4;
    float vectorValue[4];

    NodeParam() : isInt(false), isFloat(false), isString(false), isVector3(false), isVector4(false) {}
};

struct Node {
    int id;
    bool enabled;
    scene3d_node_type type;
    std::map<std::string, NodeParam> params;
    std::vector<Node> children;
};

#endif // SCENE3D_H_INCLUDED

