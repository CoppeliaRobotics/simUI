#include "Scene3D.h"

#include "XMLUtils.h"

#include "UIProxy.h"

#include <stdexcept>

#include <boost/foreach.hpp>
#include <boost/format.hpp>

//#define DEBUG_QT3D_CALLS

#ifdef DEBUG_QT3D_CALLS
#define LOG_QT3D_CALL(msg) std::cout << "QT3D: " << msg << std::endl
#else
#define LOG_QT3D_CALL(msg)
#endif

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
    typemap["camera-controller-first-person"].type = sim_ui_scene3d_node_type_camera_controller_first_person;
    typemap["camera-controller-first-person"].params["camera"].isInt = true;
    typemap["camera-controller-first-person"].params["linear-speed"].isFloat = true;
    typemap["camera-controller-first-person"].params["look-speed"].isFloat = true;
    typemap["camera-controller-first-person"].params["acceleration"].isFloat = true;
    typemap["camera-controller-first-person"].params["deceleration"].isFloat = true;
    typemap["camera-controller-orbit"].type = sim_ui_scene3d_node_type_camera_controller_orbit;
    typemap["camera-controller-orbit"].params["camera"].isInt = true;
    typemap["camera-controller-orbit"].params["linear-speed"].isFloat = true;
    typemap["camera-controller-orbit"].params["look-speed"].isFloat = true;
    typemap["camera-controller-orbit"].params["zoom-in-limit"].isFloat = true;
    typemap["light-point"].type = sim_ui_scene3d_node_type_light_point;
    typemap["light-point"].params["intensity"].isFloat = true;
    typemap["light-point"].params["constant-attenuation"].isFloat = true;
    typemap["light-point"].params["linear-attenuation"].isFloat = true;
    typemap["light-point"].params["quadratic-attenuation"].isFloat = true;
    typemap["light-point"].params["color"].isVector3 = true;
    typemap["light-directional"].type = sim_ui_scene3d_node_type_light_directional;
    typemap["light-directional"].params["intensity"].isFloat = true;
    typemap["light-directional"].params["color"].isVector3 = true;
    typemap["light-directional"].params["world-direction"].isVector3 = true;
    typemap["light-spot"].type = sim_ui_scene3d_node_type_light_spot;
    typemap["light-spot"].params["intensity"].isFloat = true;
    typemap["light-spot"].params["constant-attenuation"].isFloat = true;
    typemap["light-spot"].params["linear-attenuation"].isFloat = true;
    typemap["light-spot"].params["quadratic-attenuation"].isFloat = true;
    typemap["light-spot"].params["cutoff-angle"].isFloat = true;
    typemap["light-spot"].params["color"].isVector3 = true;
    typemap["light-spot"].params["local-direction"].isVector3 = true;
    typemap["mesh-cuboid"].type = sim_ui_scene3d_node_type_mesh_cuboid;
    typemap["mesh-cuboid"].params["x-extent"].isFloat = true;
    typemap["mesh-cuboid"].params["y-extent"].isFloat = true;
    typemap["mesh-cuboid"].params["z-extent"].isFloat = true;
    typemap["mesh-torus"].type = sim_ui_scene3d_node_type_mesh_torus;
    typemap["mesh-torus"].params["radius"].isFloat = true;
    typemap["mesh-torus"].params["minor-radius"].isFloat = true;
    typemap["mesh-torus"].params["rings"].isInt = true;
    typemap["mesh-torus"].params["slices"].isInt = true;
    typemap["mesh-cone"].type = sim_ui_scene3d_node_type_mesh_cone;
    typemap["mesh-cone"].params["top-radius"].isFloat = true;
    typemap["mesh-cone"].params["bottom-radius"].isFloat = true;
    typemap["mesh-cone"].params["length"].isFloat = true;
    typemap["mesh-cone"].params["rings"].isInt = true;
    typemap["mesh-cone"].params["slices"].isInt = true;
    typemap["mesh-cylinder"].type = sim_ui_scene3d_node_type_mesh_cylinder;
    typemap["mesh-cylinder"].params["radius"].isFloat = true;
    typemap["mesh-cylinder"].params["length"].isFloat = true;
    typemap["mesh-cylinder"].params["rings"].isInt = true;
    typemap["mesh-cylinder"].params["slices"].isInt = true;
    typemap["mesh-plane"].type = sim_ui_scene3d_node_type_mesh_plane;
    typemap["mesh-plane"].params["width"].isFloat = true;
    typemap["mesh-plane"].params["height"].isFloat = true;
    typemap["mesh-sphere"].type = sim_ui_scene3d_node_type_mesh_sphere;
    typemap["mesh-sphere"].params["radius"].isFloat = true;
    typemap["mesh-sphere"].params["rings"].isInt = true;
    typemap["mesh-sphere"].params["slices"].isInt = true;
    typemap["mesh"].type = sim_ui_scene3d_node_type_mesh;
    typemap["mesh"].params["source"].isString = true;
    typemap["material-phong"].type = sim_ui_scene3d_node_type_material_phong;
    typemap["material-phong"].params["shininess"].isFloat = true;
    typemap["material-phong"].params["ambient"].isVector3 = true;
    typemap["material-phong"].params["diffuse"].isVector3 = true;
    typemap["material-phong"].params["specular"].isVector3 = true;
    typemap["material-gooch"].type = sim_ui_scene3d_node_type_material_gooch;
    typemap["material-gooch"].params["alpha"].isFloat = true;
    typemap["material-gooch"].params["beta"].isFloat = true;
    typemap["material-gooch"].params["shininess"].isFloat = true;
    typemap["material-gooch"].params["diffuse"].isVector3 = true;
    typemap["material-gooch"].params["specular"].isVector3 = true;
    typemap["material-gooch"].params["cool-color"].isVector3 = true;
    typemap["material-gooch"].params["warm-color"].isVector3 = true;
    typemap["material-texture"].type = sim_ui_scene3d_node_type_material_texture;
    typemap["material-texture"].params["texture"].isInt = true;
    typemap["material-texture"].params["texture-offset"].isVector2 = true;
    typemap["texture2d"].type = sim_ui_scene3d_node_type_texture2d;
    typemap["texture2d"].params["texture-image"].isInt = true;
    typemap["texture-image"].type = sim_ui_scene3d_node_type_texture_image;
    typemap["texture-image"].params["source"].isString = true;
    typemap["texture-image"].params["mirrored"].isInt = true;
};

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
            else if(metaparam.isVector2)
            {
                try
                {
                    param.isVector2 = true;
                    std::vector<std::string> v;
                    xmlutils::string2vector(attrValue, v, 2, 2, ",");
                    for(int i = 0; i < 2; i++)
                        param.vectorValue[i] = boost::lexical_cast<float>(v[i]);
                }
                catch(std::exception &ex)
                {
                    throw std::runtime_error((boost::format("invalid value for param '%s' in node '%s' (expected Vector2)") % attr % tag1).str());
                }
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

    clearColor = xmlutils::getAttrIntV(e, "clear-color", "-1,-1,-1", 3, 3, ",");

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
        // exception: we don't create a new camera node, but use the already existing one
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
        else if(p.isVector2) setVector2Parameter(node.id, p.name, p.vectorValue[0], p.vectorValue[1]);
        else if(p.isVector3) setVector3Parameter(node.id, p.name, p.vectorValue[0], p.vectorValue[1], p.vectorValue[2]);
        else if(p.isVector4) setVector4Parameter(node.id, p.name, p.vectorValue[0], p.vectorValue[1], p.vectorValue[2], p.vectorValue[3]);
    }

    return qnode;
}

void dumpNodeTree(Qt3DCore::QNode *node, int level = 0)
{
    std::string ind = ""; for(int i = 0; i < level; i++) ind += "    ";
    std::string typeStr = "NODE", extra = "";
    if(Qt3DCore::QEntity *e = dynamic_cast<Qt3DCore::QEntity*>(node))
    {
        typeStr = "ENTITY";
        extra += " components=[";
        for(int i = 0; i < e->components().size(); i++)
            extra += (boost::format("%s%x") % (i ? ", " : "") % e->components()[i]).str();
        extra += "]";
    }
    else if(dynamic_cast<Qt3DCore::QTransform*>(node)) typeStr = "TRANSFORM";
    else if(dynamic_cast<Qt3DRender::QCamera*>(node)) typeStr = "CAMERA";
    else if(dynamic_cast<Qt3DExtras::QFirstPersonCameraController*>(node)) typeStr = "FIRST_PERSON_CAMERA_CONTROLLER";
    else if(dynamic_cast<Qt3DRender::QPointLight*>(node)) typeStr = "POINT_LIGHT";
    else if(dynamic_cast<Qt3DExtras::QCuboidMesh*>(node)) typeStr = "CUBOID_MESH";
    else if(dynamic_cast<Qt3DExtras::QPhongMaterial*>(node)) typeStr = "PHONG_MATERIAL";
    std::cout << ind << (boost::format("%s %x id=%d enabled=%d") % typeStr % node % node->id() % node->isEnabled()).str() << extra << std::endl;
    BOOST_FOREACH(Qt3DCore::QNode *node1, node->childNodes())
    {
        dumpNodeTree(node1, level + 1);
    }
}

QWidget * Scene3D::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    view = new Qt3DExtras::Qt3DWindow();
    view->defaultFrameGraph()->setClearColor((clearColor[0] >= 0 && clearColor[1] >= 0 && clearColor[2] >= 0) ? QColor(clearColor[0], clearColor[1], clearColor[2]) : parent->palette().color(parent->backgroundRole()));
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
    return nodeFactory(type, 0L, true);
}

Qt3DCore::QNode * Scene3D::nodeFactory(int type, Qt3DCore::QNode *parent, bool onlyTest)
{
#define MAP_NODE_TYPE(t,c) \
    case sim_ui_scene3d_node_type_##t: \
        if(onlyTest) return (Qt3DCore::QNode *)1; \
        ret = new c(parent); \
        LOG_QT3D_CALL((boost::format("new %s(%x) -> %x") % #c % (void*)parent % ret).str()); \
        break;

    Qt3DCore::QNode *ret = 0L;

    switch(type)
    {
    MAP_NODE_TYPE(entity, Qt3DCore::QEntity);
    MAP_NODE_TYPE(transform, Qt3DCore::QTransform);
    MAP_NODE_TYPE(camera, Qt3DRender::QCamera);
    MAP_NODE_TYPE(camera_controller_first_person, Qt3DExtras::QFirstPersonCameraController);
    MAP_NODE_TYPE(camera_controller_orbit, Qt3DExtras::QOrbitCameraController);
    MAP_NODE_TYPE(light_point, Qt3DRender::QPointLight);
    MAP_NODE_TYPE(light_directional, Qt3DRender::QDirectionalLight);
    MAP_NODE_TYPE(light_spot, Qt3DRender::QSpotLight);
    MAP_NODE_TYPE(mesh_cuboid, Qt3DExtras::QCuboidMesh);
    MAP_NODE_TYPE(mesh_torus, Qt3DExtras::QTorusMesh);
    MAP_NODE_TYPE(mesh_cone, Qt3DExtras::QConeMesh);
    MAP_NODE_TYPE(mesh_cylinder, Qt3DExtras::QCylinderMesh);
    MAP_NODE_TYPE(mesh_plane, Qt3DExtras::QPlaneMesh);
    MAP_NODE_TYPE(mesh_sphere, Qt3DExtras::QSphereMesh);
    MAP_NODE_TYPE(mesh, Qt3DRender::QMesh);
    MAP_NODE_TYPE(material_phong, Qt3DExtras::QPhongMaterial);
    MAP_NODE_TYPE(material_gooch, Qt3DExtras::QGoochMaterial);
    MAP_NODE_TYPE(material_texture, Qt3DExtras::QTextureMaterial);
    MAP_NODE_TYPE(texture2d, Qt3DRender::QTexture2D);
    MAP_NODE_TYPE(texture_image, Qt3DRender::QTextureImage);
    }

#undef MAP_NODE_TYPE

    return ret;
}

Qt3DCore::QNode * Scene3D::addNode(int id, int parentId, int type)
{
    if(nodeById(id))
        throw std::runtime_error((boost::format("duplicate node id: %d") % id).str());

    Qt3DCore::QNode *parent = nodeById(parentId);
    if(!parent)
        throw std::runtime_error((boost::format("invalid parent id: %d") % parentId).str());

    Qt3DCore::QNode *node = nodeFactory(type, parent, false);
    if(!node)
        throw std::runtime_error((boost::format("invalid node type: %d") % type).str());

    // if it is a QComponent, add it as a component of parent node
    if(Qt3DCore::QComponent *qcomp = dynamic_cast<Qt3DCore::QComponent*>(node))
    {
        if(Qt3DCore::QEntity *qe = dynamic_cast<Qt3DCore::QEntity*>(parent))
        {
            LOG_QT3D_CALL((boost::format("%x->addComponent(%x)") % (void*)qe % (void*)qcomp).str());
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
        throw std::runtime_error((boost::format("invalid node id: %d") % id).str());

    LOG_QT3D_CALL((boost::format("%x->setEnabled(%d)") % (void*)node % enabled).str());
    node->setEnabled(enabled);
}

#define REPORT_INVALID_PARAMETER(p) \
    std::cout << "WARNING: unknown Scne3D Node parameter: " << p << std::endl;

void Scene3D::setIntParameter(int id, std::string param, int value)
{
    Qt3DCore::QNode *node = nodeById(id);
    if(!node)
        throw std::runtime_error((boost::format("invalid node id: %d") % id).str());

    LOG_QT3D_CALL((boost::format("%x set %s = %d") % (void*)node % param % value).str());

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
    else if(Qt3DExtras::QTorusMesh *m = dynamic_cast<Qt3DExtras::QTorusMesh*>(node))
    {
        if(0) {}
        else if(param == "rings")
            m->setRings(value);
        else if(param == "slices")
            m->setSlices(value);
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else if(Qt3DExtras::QConeMesh *m = dynamic_cast<Qt3DExtras::QConeMesh*>(node))
    {
        if(0) {}
        else if(param == "rings")
            m->setRings(value);
        else if(param == "slices")
            m->setSlices(value);
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else if(Qt3DExtras::QCylinderMesh *m = dynamic_cast<Qt3DExtras::QCylinderMesh*>(node))
    {
        if(0) {}
        else if(param == "rings")
            m->setRings(value);
        else if(param == "slices")
            m->setSlices(value);
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else if(Qt3DExtras::QSphereMesh *m = dynamic_cast<Qt3DExtras::QSphereMesh*>(node))
    {
        if(0) {}
        else if(param == "rings")
            m->setRings(value);
        else if(param == "slices")
            m->setSlices(value);
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
    else if(Qt3DExtras::QOrbitCameraController *c = dynamic_cast<Qt3DExtras::QOrbitCameraController*>(node))
    {
        if(0) {}
        else if(param == "camera")
            c->setCamera(dynamic_cast<Qt3DRender::QCamera*>(nodeById(value)));
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else if(Qt3DExtras::QTextureMaterial *t = dynamic_cast<Qt3DExtras::QTextureMaterial*>(node))
    {
        if(0) {}
        else if(param == "texture")
            t->setTexture(dynamic_cast<Qt3DRender::QAbstractTexture*>(nodeById(value)));
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else if(Qt3DRender::QTexture2D *t = dynamic_cast<Qt3DRender::QTexture2D*>(node))
    {
        if(0) {}
        else if(param == "texture-image")
            t->addTextureImage(dynamic_cast<Qt3DRender::QAbstractTextureImage*>(nodeById(value)));
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else if(Qt3DRender::QTextureImage *i = dynamic_cast<Qt3DRender::QTextureImage*>(node))
    {
        if(0) {}
        else if(param == "mirrored")
            i->setMirrored(value != 0);
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
        throw std::runtime_error((boost::format("invalid node id: %d") % id).str());

    LOG_QT3D_CALL((boost::format("%x set %s = %f") % (void*)node % param % value).str());

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
    else if(Qt3DExtras::QFirstPersonCameraController *c = dynamic_cast<Qt3DExtras::QFirstPersonCameraController*>(node))
    {
        if(0) {}
        else if(param == "linear-speed")
            c->setLinearSpeed(value);
        else if(param == "look-speed")
            c->setLookSpeed(value);
        else if(param == "acceleration")
            c->setAcceleration(value);
        else if(param == "deceleration")
            c->setDeceleration(value);
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else if(Qt3DExtras::QOrbitCameraController *c = dynamic_cast<Qt3DExtras::QOrbitCameraController*>(node))
    {
        if(0) {}
        else if(param == "linear-speed")
            c->setLinearSpeed(value);
        else if(param == "look-speed")
            c->setLookSpeed(value);
        else if(param == "zoom-in-limit")
            c->setZoomInLimit(value);
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else if(Qt3DExtras::QCuboidMesh *m = dynamic_cast<Qt3DExtras::QCuboidMesh*>(node))
    {
        if(0) {}
        else if(param == "x-extent")
            m->setXExtent(value);
        else if(param == "y-extent")
            m->setYExtent(value);
        else if(param == "z-extent")
            m->setZExtent(value);
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else if(Qt3DExtras::QTorusMesh *m = dynamic_cast<Qt3DExtras::QTorusMesh*>(node))
    {
        if(0) {}
        else if(param == "radius")
            m->setRadius(value);
        else if(param == "minor-radius")
            m->setMinorRadius(value);
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else if(Qt3DExtras::QConeMesh *m = dynamic_cast<Qt3DExtras::QConeMesh*>(node))
    {
        if(0) {}
        else if(param == "length")
            m->setLength(value);
        else if(param == "top-radius")
            m->setTopRadius(value);
        else if(param == "bottom-radius")
            m->setBottomRadius(value);
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else if(Qt3DExtras::QCylinderMesh *m = dynamic_cast<Qt3DExtras::QCylinderMesh*>(node))
    {
        if(0) {}
        else if(param == "radius")
            m->setRadius(value);
        else if(param == "length")
            m->setLength(value);
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else if(Qt3DExtras::QPlaneMesh *m = dynamic_cast<Qt3DExtras::QPlaneMesh*>(node))
    {
        if(0) {}
        else if(param == "width")
            m->setWidth(value);
        else if(param == "height")
            m->setHeight(value);
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else if(Qt3DExtras::QSphereMesh *m = dynamic_cast<Qt3DExtras::QSphereMesh*>(node))
    {
        if(0) {}
        else if(param == "radius")
            m->setRadius(value);
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
    else if(Qt3DRender::QDirectionalLight *l = dynamic_cast<Qt3DRender::QDirectionalLight*>(node))
    {
        if(0) {}
        else if(param == "intensity")
            l->setIntensity(value);
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else if(Qt3DRender::QSpotLight *l = dynamic_cast<Qt3DRender::QSpotLight*>(node))
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
        else if(param == "cutoff-angle")
            l->setCutOffAngle(value);
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
    else if(Qt3DExtras::QGoochMaterial *m = dynamic_cast<Qt3DExtras::QGoochMaterial*>(node))
    {
        if(0) {}
        else if(param == "shininess")
            m->setShininess(value);
        else if(param == "alpha")
            m->setAlpha(value);
        else if(param == "alpha")
            m->setBeta(value);
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
        throw std::runtime_error((boost::format("invalid node id: %d") % id).str());

    LOG_QT3D_CALL((boost::format("%x set %s = %s") % (void*)node % param % value).str());

    if(0) {}
    else if(Qt3DRender::QMesh *m = dynamic_cast<Qt3DRender::QMesh*>(node))
    {
        if(0) {}
        else if(param == "source")
            m->setSource(QUrl(QString::fromStdString(value)));
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else if(Qt3DRender::QTextureImage *i = dynamic_cast<Qt3DRender::QTextureImage*>(node))
    {
        if(0) {}
        else if(param == "source")
            i->setSource(QUrl(QString::fromStdString(value)));
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else
        REPORT_INVALID_PARAMETER(param);
}

void Scene3D::setVector2Parameter(int id, std::string param, float x, float y)
{
    Qt3DCore::QNode *node = nodeById(id);
    if(!node)
        throw std::runtime_error((boost::format("invalid node id: %d") % id).str());

    LOG_QT3D_CALL((boost::format("%x set %s = <%f, %f, %f>") % (void*)node % param % x % y % z).str());

    QVector2D value(x, y);

    if(0) {}
    else if(Qt3DExtras::QTextureMaterial *t = dynamic_cast<Qt3DExtras::QTextureMaterial*>(node))
    {
        if(0) {}
        else if(param == "texture-offset")
            t->setTextureOffset(value);
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else
        REPORT_INVALID_PARAMETER(param);
}

void Scene3D::setVector3Parameter(int id, std::string param, float x, float y, float z)
{
    Qt3DCore::QNode *node = nodeById(id);
    if(!node)
        throw std::runtime_error((boost::format("invalid node id: %d") % id).str());

    LOG_QT3D_CALL((boost::format("%x set %s = <%f, %f, %f>") % (void*)node % param % x % y % z).str());

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
    else if(Qt3DRender::QDirectionalLight *l = dynamic_cast<Qt3DRender::QDirectionalLight*>(node))
    {
        if(0) {}
        else if(param == "color")
            l->setColor(QColor(x, y, z));
        else if(param == "world-direction")
            l->setWorldDirection(value);
        else
            REPORT_INVALID_PARAMETER(param);
    }
    else if(Qt3DRender::QSpotLight *l = dynamic_cast<Qt3DRender::QSpotLight*>(node))
    {
        if(0) {}
        else if(param == "color")
            l->setColor(QColor(x, y, z));
        else if(param == "local-direction")
            l->setLocalDirection(value);
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
    else if(Qt3DExtras::QGoochMaterial *m = dynamic_cast<Qt3DExtras::QGoochMaterial*>(node))
    {
        if(0) {}
        else if(param == "cool-color")
            m->setCool(QColor(x, y, z));
        else if(param == "warm-color")
            m->setWarm(QColor(x, y, z));
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
        throw std::runtime_error((boost::format("invalid node id: %d") % id).str());

    LOG_QT3D_CALL((boost::format("%x set %s = <%f, %f, %f, %f>") % (void*)node % param % x % y % z % w).str());

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
