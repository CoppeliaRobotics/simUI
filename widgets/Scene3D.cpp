#include "Scene3D.h"

#include "XMLUtils.h"

#include "UIProxy.h"

#include <stdexcept>

#include <boost/foreach.hpp>
#include <boost/format.hpp>

Scene3D::Scene3D()
    : Widget("scene3d"),
      view(0L),
      rootEntity(0L)
{
}

Scene3D::~Scene3D()
{
}

static std::map<std::string, Node> typemap;

void initTypemap()
{
    typemap["entity"].type = sim_ui_scene3d_node_type_entity;
    typemap["transform"].type = sim_ui_scene3d_node_type_transform;
    typemap["transform"].params["scale"].isFloat = true;
    typemap["transform"].params["rotation-x"].isFloat = true;
    typemap["transform"].params["rotation-y"].isFloat = true;
    typemap["transform"].params["rotation-z"].isFloat = true;
    typemap["transform"].params["translation"].isVector3 = true;
    typemap["transform"].params["scale3d"].isVector3 = true;
    typemap["transform"].params["rotation"].isVector4 = true;
    typemap["camera"].type = sim_ui_scene3d_node_type_camera;
    typemap["camera"].params["projection-type"].isInt = true;
    typemap["camera"].params["orthographic-projection"].isInt = true;
    typemap["camera"].params["perspective-projection"].isInt = true;
    typemap["camera"].params["frustum-projection"].isInt = true;
    typemap["camera"].params["aspect-ratio"].isFloat = true;
    typemap["camera"].params["bottom"].isFloat = true;
    typemap["camera"].params["exposure"].isFloat = true;
    typemap["camera"].params["far-plane"].isFloat = true;
    typemap["camera"].params["field-of-view"].isFloat = true;
    typemap["camera"].params["left"].isFloat = true;
    typemap["camera"].params["near-plane"].isFloat = true;
    typemap["camera"].params["right"].isFloat = true;
    typemap["camera"].params["top"].isFloat = true;
    typemap["camera"].params["position"].isVector3 = true;
    typemap["camera"].params["up-vector"].isVector3 = true;
    typemap["camera"].params["view-center"].isVector3 = true;
    typemap["first-person-camera-controller"].type = sim_ui_scene3d_node_type_first_person_camera_controller;
    typemap["first-person-camera-controller"].params["camera"].isInt = true;
    typemap["point-light"].type = sim_ui_scene3d_node_type_point_light;
    typemap["point-light"].params["intensity"].isFloat = true;
    typemap["point-light"].params["constant-attenuation"].isFloat = true;
    typemap["point-light"].params["linear-attenuation"].isFloat = true;
    typemap["point-light"].params["quadratic-attenuation"].isFloat = true;
    typemap["point-light"].params["color"].isVector3 = true;
    typemap["mesh-cuboid"].type = sim_ui_scene3d_node_type_mesh_cuboid;
    typemap["phong-material"].type = sim_ui_scene3d_node_type_phong_material;
    typemap["phong-material"].params["shininess"].isFloat = true;
    typemap["phong-material"].params["ambient"].isVector3 = true;
    typemap["phong-material"].params["diffuse"].isVector3 = true;
    typemap["phong-material"].params["specular"].isVector3 = true;
};

int tagname2type(std::string tagName)
{
    return 0;
}

void parseNodes(tinyxml2::XMLElement *e, std::vector<Node> &nodes)
{
    static int autoId = -1;
    for(tinyxml2::XMLElement *e1 = e->FirstChildElement(); e1; e1 = e1->NextSiblingElement())
    {
        std::string tag1(e1->Value());
        std::map<std::string, Node>::iterator it = typemap.find(tag1);
        if(it == typemap.end())
            throw std::runtime_error((boost::format("invalid node type '%s'") % tag1).str());
        Node &meta = it->second;
        Node node;
        node.type = meta.type;
        node.id = -1;
        for(const tinyxml2::XMLAttribute *a = e1->FirstAttribute(); a; a = a->Next())
        {
            std::string attr(a->Name()), attrValue(a->Value());
            if(attr == "id")
            {
                node.id = boost::lexical_cast<int>(attrValue);
                continue;
            }
            if(attr == "enabled")
            {
                if(attrValue != "true" && attrValue != "false")
                    throw std::runtime_error("invalid value for paramater 'enabled'. mmust be 'true' or 'false'.");
                node.enabled = attrValue == "true";
                continue;
            }
            std::map<std::string, NodeParam>::iterator it = meta.params.find(attr);
            if(it == meta.params.end())
                throw std::runtime_error((boost::format("invalid node param '%s' for node '%s'") % attr % tag1).str());
            NodeParam &metaparam = it->second;
            NodeParam param;
            param.name = attr;
            if(metaparam.isInt)
            {
                try
                {
                    param.isInt = true;
                    param.intValue = boost::lexical_cast<int>(attrValue);
                }
                catch(std::exception &ex)
                {
                    throw std::runtime_error((boost::format("invalid value for param '%s' in node '%s' (expected int)") % attr % tag1).str());
                }
            }
            else if(metaparam.isFloat)
            {
                try
                {
                    param.isFloat = true;
                    param.floatValue = boost::lexical_cast<float>(attrValue);
                }
                catch(std::exception &ex)
                {
                    throw std::runtime_error((boost::format("invalid value for param '%s' in node '%s' (expected float)") % attr % tag1).str());
                }
            }
            else if(metaparam.isString)
            {
                param.isString = true;
                param.stringValue = attrValue;
            }
            else if(metaparam.isVector3)
            {
                try
                {
                    param.isVector3 = true;
                    std::vector<std::string> v;
                    xmlutils::string2vector(attrValue, v, 3, 3, ",");
                    for(int i = 0; i < 3; i++)
                        param.vectorValue[i] = boost::lexical_cast<float>(v[i]);
                }
                catch(std::exception &ex)
                {
                    throw std::runtime_error((boost::format("invalid value for param '%s' in node '%s' (expected Vector3)") % attr % tag1).str());
                }
            }
            else if(metaparam.isVector4)
            {
                try
                {
                    param.isVector4 = true;
                    std::vector<std::string> v;
                    xmlutils::string2vector(attrValue, v, 4, 4, ",");
                    for(int i = 0; i < 4; i++)
                        param.vectorValue[i] = boost::lexical_cast<float>(v[i]);
                }
                catch(std::exception &ex)
                {
                    throw std::runtime_error((boost::format("invalid value for param '%s' in node '%s' (expected Vector4)") % attr % tag1).str());
                }
            }
            node.params[attr] = param;
        }
        if(node.id == -1)
            node.id = --autoId;
        parseNodes(e1, node.children);
        nodes.push_back(node);
    }
}

void Scene3D::parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    if(typemap.size() == 0) initTypemap();

    Widget::parse(parent, widgets, e);

    clearColor = xmlutils::getAttrIntV(e, "clear-color", "0,0,0", 3, 3, ",");

    parseNodes(e, nodes);
}

void Scene3D::createNodes(Qt3DCore::QNode *parentQNode, int parentId, std::vector<Node> &nodes)
{
    BOOST_FOREACH(Node &node, nodes)
    {
        Qt3DCore::QNode *qnode = createNode(parentQNode, parentId, node);
        createNodes(qnode, node.id, node.children);
    }
}

Qt3DCore::QNode * Scene3D::createNode(Qt3DCore::QNode *parentQNode, int parentId, Node &node)
{
    Qt3DCore::QNode *qnode;

    if(node.type == sim_ui_scene3d_node_type_camera)
    {
        // exception: we don't create camare, but use the already existing camera
        Qt3DCore::QNode *cameraNode = view->camera();
        nodeById_[node.id] = cameraNode;
        qnode = cameraNode;
    }
    else
    {
        qnode = addNode(node.id, parentId, node.type);
    }

    qnode->setEnabled(node.enabled);

    for(std::map<std::string, NodeParam>::iterator it = node.params.begin(); it != node.params.end(); ++it)
    {
        const NodeParam &p = it->second;
        if(p.isInt) setIntParameter(node.id, p.name, p.intValue);
        else if(p.isFloat) setFloatParameter(node.id, p.name, p.floatValue);
        else if(p.isString) setStringParameter(node.id, p.name, p.stringValue);
        else if(p.isVector3) setVector3Parameter(node.id, p.name, p.vectorValue[0], p.vectorValue[1], p.vectorValue[2]);
        else if(p.isVector4) setVector4Parameter(node.id, p.name, p.vectorValue[0], p.vectorValue[1], p.vectorValue[2], p.vectorValue[3]);
    }

    return qnode;
}

QWidget * Scene3D::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    view = new Qt3DExtras::Qt3DWindow();
    view->defaultFrameGraph()->setClearColor(QColor(clearColor[0], clearColor[1], clearColor[2]));
    QWidget *container = QWidget::createWindowContainer(view);
    QSize screenSize = view->screen()->size();
    container->setMinimumSize(QSize(200, 100));
    container->setMaximumSize(screenSize);

    Qt3DInput::QInputAspect *input = new Qt3DInput::QInputAspect;
    view->registerAspect(input);

    rootEntity = new Qt3DCore::QEntity();
    view->setRootEntity(rootEntity);

    createNodes(rootEntity, -1, nodes);

    QObject::connect(view->camera(), &Qt3DRender::QCamera::viewCenterChanged, uiproxy, &UIProxy::onViewCenterChanged);
    QObject::connect(view->camera(), &Qt3DRender::QCamera::positionChanged, uiproxy, &UIProxy::onPositionChanged);

    setQWidget(container);
    setProxy(proxy);
    return container;
}

Qt3DCore::QNode * Scene3D::nodeById(int id)
{
    if(id == -1) return rootEntity;
    std::map<int, Qt3DCore::QNode*>::iterator it = nodeById_.find(id);
    if(it == nodeById_.end()) return 0L;
    else return it->second;
}

bool Scene3D::nodeExists(int id)
{
    if(id == -1) return true;
    std::map<int, Qt3DCore::QNode*>::iterator it = nodeById_.find(id);
    return it != nodeById_.end();
}

bool Scene3D::nodeTypeIsValid(int type)
{
    Qt3DCore::QNode *node = nodeFactory(type);
    if(node)
    {
        delete node;
        return true;
    }
    else return false;
}

Qt3DCore::QNode * Scene3D::nodeFactory(int type)
{
#define MAP_NODE_TYPE(t,c) case sim_ui_scene3d_node_type_##t: return new c(parent);
    switch(type)
    {
    MAP_NODE_TYPE(entity, Qt3DCore::QEntity);
    MAP_NODE_TYPE(transform, Qt3DCore::QTransform);
    MAP_NODE_TYPE(camera, Qt3DRender::QCamera);
    MAP_NODE_TYPE(first_person_camera_controller, Qt3DExtras::QFirstPersonCameraController);
    MAP_NODE_TYPE(point_light, Qt3DRender::QPointLight);
    MAP_NODE_TYPE(mesh_cuboid, Qt3DExtras::QCuboidMesh);
    MAP_NODE_TYPE(phong_material, Qt3DExtras::QPhongMaterial);
    }
#undef MAP_NODE_TYPE
    return 0L;
}

Qt3DCore::QNode * Scene3D::addNode(int id, int parentId, int type)
{
    if(nodeById(id))
        throw std::runtime_error("duplicate node id");

    Qt3DCore::QNode *parent = nodeById(parentId);
    if(!parent)
        throw std::runtime_error("invalid parent id");

    Qt3DCore::QNode *node = nodeFactory(type);
    if(!node)
        throw std::runtime_error((boost::format("invalid node type: %d") % type).str());

    // if it is a QComponent, add it as a component of parent node
    if(Qt3DCore::QComponent *qcomp = dynamic_cast<Qt3DCore::QComponent*>(node))
    {
        if(Qt3DCore::QEntity *qe = dynamic_cast<Qt3DCore::QEntity*>(parent))
        {
            qe->addComponent(qcomp);
        }
    }

    nodeById_[id] = node;

    return node;
}

void Scene3D::removeNode(int id)
{
}

void Scene3D::enableNode(int id, bool enabled)
{
    Qt3DCore::QNode *node = nodeById(id);
    if(!node)
        throw std::runtime_error("invalid node id");

    node->setEnabled(enabled);
}

#define REPORT_INVALID_PARAMETER(p) \
    std::cout << "WARNING: unknown Scne3D Node parameter: " << p << std::endl;

void Scene3D::setIntParameter(int id, std::string param, int value)
{
    Qt3DCore::QNode *node = nodeById(id);
    if(!node)
        throw std::runtime_error("invalid node id");

    if(0) {}
    else if(Qt3DRender::QCamera *c = dynamic_cast<Qt3DRender::QCamera*>(node))
    {
        if(0) {}
        else if(param == "projection-type")
            c->setProjectionType((Qt3DRender::QCameraLens::ProjectionType)value);
        else if(param == "orthographic-projection")
            c->setProjectionType(Qt3DRender::QCameraLens::OrthographicProjection);
        else if(param == "perspective-projection")
            c->setProjectionType(Qt3DRender::QCameraLens::PerspectiveProjection);
        else if(param == "frustum-projection")
            c->setProjectionType(Qt3DRender::QCameraLens::FrustumProjection);
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else if(Qt3DExtras::QFirstPersonCameraController *c = dynamic_cast<Qt3DExtras::QFirstPersonCameraController*>(node))
    {
        if(0) {}
        else if(param == "camera")
            c->setCamera(dynamic_cast<Qt3DRender::QCamera*>(nodeById(value)));
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else
        REPORT_INVALID_PARAMETER(param);
}

void Scene3D::setFloatParameter(int id, std::string param, float value)
{
    Qt3DCore::QNode *node = nodeById(id);
    if(!node)
        throw std::runtime_error("invalid node id");

    if(0) {}
    else if(Qt3DCore::QTransform *t = dynamic_cast<Qt3DCore::QTransform*>(node))
    {
        if(0) {}
        else if(param == "scale")
            t->setScale(value);
        else if(param == "rotation-x")
            t->setRotationX(value);
        else if(param == "rotation-y")
            t->setRotationY(value);
        else if(param == "rotation-z")
            t->setRotationZ(value);
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else if(Qt3DRender::QCamera *c = dynamic_cast<Qt3DRender::QCamera*>(node))
    {
        if(0) {}
        else if(param == "aspect-ratio")
            c->setAspectRatio(value);
        else if(param == "bottom")
            c->setBottom(value);
        else if(param == "exposure")
            c->setExposure(value);
        else if(param == "far-plane")
            c->setFarPlane(value);
        else if(param == "field-of-view")
            c->setFieldOfView(value);
        else if(param == "left")
            c->setLeft(value);
        else if(param == "near-plane")
            c->setNearPlane(value);
        else if(param == "right")
            c->setRight(value);
        else if(param == "top")
            c->setTop(value);
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else if(Qt3DRender::QPointLight *l = dynamic_cast<Qt3DRender::QPointLight*>(node))
    {
        if(0) {}
        else if(param == "intensity")
            l->setIntensity(value);
        else if(param == "constant-attenuation")
            l->setConstantAttenuation(value);
        else if(param == "linear-attenuation")
            l->setLinearAttenuation(value);
        else if(param == "quadratic-attenuation")
            l->setQuadraticAttenuation(value);
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else if(Qt3DExtras::QPhongMaterial *m = dynamic_cast<Qt3DExtras::QPhongMaterial*>(node))
    {
        if(0) {}
        else if(param == "shininess")
            m->setShininess(value);
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else
        REPORT_INVALID_PARAMETER(param);
}

void Scene3D::setStringParameter(int id, std::string param, std::string value)
{
    Qt3DCore::QNode *node = nodeById(id);
    if(!node)
        throw std::runtime_error("invalid node id");

    if(0) {}
    else
        REPORT_INVALID_PARAMETER(param);
}

void Scene3D::setVector3Parameter(int id, std::string param, float x, float y, float z)
{
    Qt3DCore::QNode *node = nodeById(id);
    if(!node)
        throw std::runtime_error("invalid node id");

    QVector3D value(x, y, z);

    if(0) {}
    else if(Qt3DCore::QTransform *t = dynamic_cast<Qt3DCore::QTransform*>(node))
    {
        if(0) {}
        else if(param == "translation")
            t->setTranslation(value);
        else if(param == "scale3d")
            t->setScale3D(value);
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else if(Qt3DRender::QCamera *c = dynamic_cast<Qt3DRender::QCamera*>(node))
    {
        if(0) {}
        else if(param == "position")
            c->setPosition(value);
        else if(param == "up-vector")
            c->setUpVector(value);
        else if(param == "view-center")
            c->setViewCenter(value);
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else if(Qt3DRender::QPointLight *l = dynamic_cast<Qt3DRender::QPointLight*>(node))
    {
        if(0) {}
        else if(param == "color")
            l->setColor(QColor(x, y, z));
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else if(Qt3DExtras::QPhongMaterial *m = dynamic_cast<Qt3DExtras::QPhongMaterial*>(node))
    {
        if(0) {}
        else if(param == "ambient")
            m->setAmbient(QColor(x, y, z));
        else if(param == "diffuse")
            m->setDiffuse(QColor(x, y, z));
        else if(param == "specular")
            m->setSpecular(QColor(x, y, z));
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else
        REPORT_INVALID_PARAMETER(param);
}

void Scene3D::setVector4Parameter(int id, std::string param, float x, float y, float z, float w)
{
    Qt3DCore::QNode *node = nodeById(id);
    if(!node)
        throw std::runtime_error("invalid node id");

    if(0) {}
    else if(Qt3DCore::QTransform *t = dynamic_cast<Qt3DCore::QTransform*>(node))
    {
        if(0) {}
        else if(param == "rotation")
            t->setRotation(QQuaternion(w, x, y, z));
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else
        REPORT_INVALID_PARAMETER(param);
}

