#ifndef SCENE3D_H_INCLUDED
#define SCENE3D_H_INCLUDED

#include <vector>
#include <map>
#include <string>

#include <QWidget>

#include <QtCore/QObject>

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraLens>
#include <Qt3DInput/QInputAspect>
#include <Qt3DCore/QAspectEngine>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QTorusMesh>
#include <Qt3DExtras/QConeMesh>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QPlaneMesh>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DRender/QMesh>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QMaterial>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QGoochMaterial>
#include <Qt3DExtras/QTextureMaterial>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QSceneLoader>
#include <Qt3DRender/QPointLight>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DRender/QSpotLight>
#include <Qt3DRender/QRenderAspect>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QFirstPersonCameraController>
#include <Qt3DExtras/QOrbitCameraController>

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

    Qt3DCore::QNode * nodeFactory(int type, Qt3DCore::QNode *parent, bool onlyTest);

public:
    Scene3D();
    virtual ~Scene3D();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    Qt3DCore::QNode * nodeById(int id);
    bool nodeExists(int id);
    bool nodeTypeIsValid(int type);

    Qt3DCore::QNode * addNode(int id, int parentId, int type);
    void removeNode(int id);
    void enableNode(int id, bool enabled);
    void setIntParameter(int id, std::string param, int value);
    void setFloatParameter(int id, std::string param, float value);
    void setStringParameter(int id, std::string param, std::string value);
    void setVector2Parameter(int id, std::string param, float x, float y);
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
    bool isVector2;
    bool isVector3;
    bool isVector4;
    float vectorValue[4];

    NodeParam() : isInt(false), isFloat(false), isString(false), isVector2(false), isVector3(false), isVector4(false) {}
};

struct Node {
    int id;
    bool enabled;
    scene3d_node_type type;
    std::map<std::string, NodeParam> params;
    std::vector<Node> children;

    Node() : id(0), enabled(true) {}
};

#endif // SCENE3D_H_INCLUDED

