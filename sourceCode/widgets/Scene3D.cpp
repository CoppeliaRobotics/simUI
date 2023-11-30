#include "Scene3D.h"

#include "XMLUtils.h"

#include "UI.h"

#include <stdexcept>

#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

//#define DEBUG_QT3D_CALLS

#ifdef DEBUG_QT3D_CALLS
#define LOG_QT3D_CALL(msg) log(sim_verbosity_debug, boost::format("QT3D: %s") % msg)
#else
#define LOG_QT3D_CALL(msg)
#endif

std::map<Qt3DCore::QNode*, Scene3D*> Scene3D::nodeScene_;

void NodeParam::parse(const std::string &nodeName, const std::string &s, NodeParam *dest)
{
    std::string expected = "";
    try
    {
        if(isInt)
        {
            expected = "int";
            dest->intValue = boost::lexical_cast<int>(s);
            dest->isInt = true;
        }
        else if(isFloat)
        {
            expected = "float";
            dest->floatValue = boost::lexical_cast<double>(s);
            dest->isFloat = true;
        }
        else if(isString)
        {
            expected = "string";
            dest->isString = true;
            dest->stringValue = s;
        }
        else if(isVector2)
        {
            expected = "vector2";
            std::vector<std::string> v;
            xmlutils::string2vector(s, v, 2, 2, ",");
            for(int i = 0; i < 2; i++)
                dest->vectorValue[i] = boost::lexical_cast<double>(v[i]);
            dest->isVector2 = true;
        }
        else if(isVector3)
        {
            expected = "vector3";
            std::vector<std::string> v;
            xmlutils::string2vector(s, v, 3, 3, ",");
            for(int i = 0; i < 3; i++)
                dest->vectorValue[i] = boost::lexical_cast<double>(v[i]);
            dest->isVector3 = true;
        }
        else if(isVector4)
        {
            expected = "vector4";
            std::vector<std::string> v;
            xmlutils::string2vector(s, v, 4, 4, ",");
            for(int i = 0; i < 4; i++)
                dest->vectorValue[i] = boost::lexical_cast<double>(v[i]);
            dest->isVector4 = true;
        }
    }
    catch(std::exception &ex)
    {
        throw std::runtime_error((boost::format("invalid value for param '%s' in node '%s' (expected %s)") % dest->name % nodeName % expected).str());
    }
}

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
#define TYPE(xmltag,enumitem,c,postinit) typemap[xmltag].type = simui_scene3d_node_type_##enumitem; { std::map<std::string, NodeParam> &params_ = typemap[xmltag].params;
#define ENDTYPE }
#define PARAM_INT(xmlattr,m) params_[xmlattr].isInt = true;
#define PARAM_FLOAT(xmlattr,m) params_[xmlattr].isFloat = true;
#define PARAM_STRING(xmlattr,m) params_[xmlattr].isString = true;
#define PARAM_VEC2(xmlattr,m) params_[xmlattr].isVector2 = true;
#define PARAM_VEC3(xmlattr,m) params_[xmlattr].isVector3 = true;
#define PARAM_VEC4(xmlattr,m) params_[xmlattr].isVector4 = true;
#include "Scene3D.Qt3DWrapper.cpp"
#undef TYPE
#undef ENDTYPE
#undef PARAM_INT
#undef PARAM_FLOAT
#undef PARAM_STRING
#undef PARAM_VEC2
#undef PARAM_VEC3
#undef PARAM_VEC4
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
                try
                {
                    node.id = boost::lexical_cast<int>(attrValue);
                    if(node.id <= 0) throw std::runtime_error("id must be positive");
                }
                catch(std::exception &ex)
                {
                    throw std::runtime_error("invalid value for paramater 'id'. mmust be a positive integer.");
                }
            }
            else if(attr == "enabled")
            {
                if(attrValue != "true" && attrValue != "false")
                    throw std::runtime_error("invalid value for paramater 'enabled'. mmust be 'true' or 'false'.");
                node.enabled = attrValue == "true";
            }
            else
            {
                std::map<std::string, NodeParam>::iterator it = meta.params.find(attr);
                if(it == meta.params.end())
                    throw std::runtime_error((boost::format("invalid node param '%s' for node '%s'") % attr % tag1).str());
                NodeParam &metaparam = it->second;
                node.params[attr].name = attr;
                metaparam.parse(tag1, attrValue, &node.params[attr]);
            }
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

    onClick = xmlutils::getAttrStr(e, "on-click", "");

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

    if(node.type == simui_scene3d_node_type_camera)
    {
        // exception: we don't create a new camera node, but use the already existing one
        Qt3DCore::QNode *cameraNode = view->camera();
        nodeById_[node.id] = cameraNode;
        nodeId_[cameraNode] = node.id;
        Scene3D::nodeScene_[cameraNode] = this;
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
        typeStr = "Qt3DCore::QEntity";
        extra += " components=[";
        for(int i = 0; i < e->components().size(); i++)
            extra += (boost::format("%s%x") % (i ? ", " : "") % e->components()[i]).str();
        extra += "]";
    }
#define TYPE(xmltag,enumitem,c,postinit) else if(dynamic_cast<c*>(node)) typeStr = #c;
#define ENDTYPE
#define PARAM_INT(xmlattr,m)
#define PARAM_FLOAT(xmlattr,m)
#define PARAM_STRING(xmlattr,m)
#define PARAM_VEC2(xmlattr,m)
#define PARAM_VEC3(xmlattr,m)
#define PARAM_VEC4(xmlattr,m)
#include "Scene3D.Qt3DWrapper.cpp"
#undef TYPE
#undef ENDTYPE
#undef PARAM_INT
#undef PARAM_FLOAT
#undef PARAM_STRING
#undef PARAM_VEC2
#undef PARAM_VEC3
#undef PARAM_VEC4
    std::cout << ind << (boost::format("%s %x enabled=%d") % typeStr % node % node->isEnabled()).str() << extra << std::endl;
    BOOST_FOREACH(Qt3DCore::QNode *node1, node->childNodes())
    {
        dumpNodeTree(node1, level + 1);
    }
}

QWidget * Scene3D::createQtWidget(Proxy *proxy, UI *ui, QWidget *parent)
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

    QObject::connect(view->camera(), &Qt3DRender::QCamera::viewCenterChanged, ui, &UI::onViewCenterChanged);
    QObject::connect(view->camera(), &Qt3DRender::QCamera::positionChanged, ui, &UI::onPositionChanged);

    setQWidget(container);
    setProxy(proxy);
    return container;
}

Qt3DCore::QNode * Scene3D::nodeById(int id, Qt3DCore::QNode *def)
{
    if(id == -1) return rootEntity;
    std::map<int, Qt3DCore::QNode*>::iterator it = nodeById_.find(id);
    if(it == nodeById_.end()) return def;
    else return it->second;
}

Qt3DCore::QNode * Scene3D::nodeById(int id)
{
    if(id == -1) return rootEntity;
    std::map<int, Qt3DCore::QNode*>::iterator it = nodeById_.find(id);
    if(it == nodeById_.end()) throw std::runtime_error((boost::format("invalid node id: %d") % id).str());
    return it->second;
}

int Scene3D::nodeId(Qt3DCore::QNode *node)
{
    return nodeId_[node];
}

Scene3D * Scene3D::nodeScene(Qt3DCore::QNode *node)
{
    return Scene3D::nodeScene_[node];
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
    switch(type)
    {
#define TYPE(xmltag,enumitem,c,postinit) \
    case simui_scene3d_node_type_##enumitem: { \
        if(onlyTest) return (Qt3DCore::QNode *)1; \
        c *o = new c(parent); \
        postinit \
        LOG_QT3D_CALL((boost::format("new %s(%x) -> %x") % #c % (void*)parent % ret).str()); \
        return o; \
    } break;
#define ENDTYPE
#define PARAM_INT(xmlattr,m)
#define PARAM_FLOAT(xmlattr,m)
#define PARAM_STRING(xmlattr,m)
#define PARAM_VEC2(xmlattr,m)
#define PARAM_VEC3(xmlattr,m)
#define PARAM_VEC4(xmlattr,m)
#include "Scene3D.Qt3DWrapper.cpp"
#undef TYPE
#undef ENDTYPE
#undef PARAM_INT
#undef PARAM_FLOAT
#undef PARAM_STRING
#undef PARAM_VEC2
#undef PARAM_VEC3
#undef PARAM_VEC4
    }

    return 0L;
}

Qt3DCore::QNode * Scene3D::addNode(int id, int parentId, int type)
{
    if(nodeById(id, 0L))
        throw std::runtime_error((boost::format("duplicate node id: %d") % id).str());

    Qt3DCore::QNode *parent = nodeById(parentId, 0L);
    if(!parent)
        throw std::runtime_error((boost::format("invalid parent id: %d") % parentId).str());

    Qt3DCore::QNode *node = 0L;

    try
    {
        node = nodeFactory(type, parent, false);
        if(!node)
            throw std::runtime_error((boost::format("invalid node type: %d") % type).str());
    }
    catch(std::exception &ex)
    {
        std::cout << "ERROR: failed to create object " << id << " of type " << scene3d_node_type_string((scene3d_node_type)type) << ": " << ex.what() << std::endl;
        return 0L;
    }

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
    nodeId_[node] = id;
    Scene3D::nodeScene_[node] = this;

    return node;
}

void Scene3D::removeNode(int id)
{
}

void Scene3D::enableNode(int id, bool enabled)
{
    Qt3DCore::QNode *node = nodeById(id);

    LOG_QT3D_CALL((boost::format("%x->setEnabled(%d)") % (void*)node % enabled).str());
    node->setEnabled(enabled);
}

#define REPORT_INVALID_PARAMETER(p) \
    std::cout << "WARNING: unknown Scne3D Node parameter: " << p << std::endl;

#define BEGIN try { if(0) {
#define END } else REPORT_INVALID_PARAMETER(param); } catch(std::exception &ex) {std::cout << "ERROR: failed to set param '" << param << "' of node " << id << ": " << ex.what() << std::endl;}
#define TYPE(xmltag,enumitem,c,postinit) } else if(c *o = dynamic_cast<c*>(node)) { if(0){
#define ENDTYPE } else REPORT_INVALID_PARAMETER(param);
#define PARAM_ON(x,m) } else if(param == x) { m
#define PARAM_OFF
#define PARAM_INT(xmlattr,m) PARAM_OFF
#define PARAM_FLOAT(xmlattr,m) PARAM_OFF
#define PARAM_STRING(xmlattr,m) PARAM_OFF
#define PARAM_VEC2(xmlattr,m) PARAM_OFF
#define PARAM_VEC3(xmlattr,m) PARAM_OFF
#define PARAM_VEC4(xmlattr,m) PARAM_OFF

void Scene3D::setIntParameter(int id, std::string param, int value)
{
    Qt3DCore::QNode *node = nodeById(id);

    LOG_QT3D_CALL((boost::format("%x set %s = %d") % (void*)node % param % value).str());

    BEGIN
#undef PARAM_INT
#define PARAM_INT(xmlattr,m) PARAM_ON(xmlattr,m)
#include "Scene3D.Qt3DWrapper.cpp"
#undef PARAM_INT
#define PARAM_INT(xmlattr,m) PARAM_OFF
    END
}

void Scene3D::setFloatParameter(int id, std::string param, double value)
{
    Qt3DCore::QNode *node = nodeById(id);

    LOG_QT3D_CALL((boost::format("%x set %s = %f") % (void*)node % param % value).str());

    BEGIN
#undef PARAM_FLOAT
#define PARAM_FLOAT(xmlattr,m) PARAM_ON(xmlattr,m)
#include "Scene3D.Qt3DWrapper.cpp"
#undef PARAM_FLOAT
#define PARAM_FLOAT(xmlattr,m) PARAM_OFF
    END
}

void Scene3D::setStringParameter(int id, std::string param, std::string value)
{
    Qt3DCore::QNode *node = nodeById(id);

    LOG_QT3D_CALL((boost::format("%x set %s = %s") % (void*)node % param % value).str());

    QString qstring(QString::fromStdString(value));
    QUrl url(qstring);

    BEGIN
#undef PARAM_STRING
#define PARAM_STRING(xmlattr,m) PARAM_ON(xmlattr,m)
#include "Scene3D.Qt3DWrapper.cpp"
#undef PARAM_STRING
#define PARAM_STRING(xmlattr,m) PARAM_OFF
    END
}

void Scene3D::setVector2Parameter(int id, std::string param, double x, double y)
{
    Qt3DCore::QNode *node = nodeById(id);

    LOG_QT3D_CALL((boost::format("%x set %s = <%f, %f, %f>") % (void*)node % param % x % y % z).str());

    QVector2D vector(x, y);

    BEGIN
#undef PARAM_VEC2
#define PARAM_VEC2(xmlattr,m) PARAM_ON(xmlattr,m)
#include "Scene3D.Qt3DWrapper.cpp"
#undef PARAM_VEC2
#define PARAM_VEC2(xmlattr,m) PARAM_OFF
    END
}

void Scene3D::setVector3Parameter(int id, std::string param, double x, double y, double z)
{
    Qt3DCore::QNode *node = nodeById(id);

    LOG_QT3D_CALL((boost::format("%x set %s = <%f, %f, %f>") % (void*)node % param % x % y % z).str());

    QVector3D vector(x, y, z);
    QColor color((int)x, (int)y, (int)z);

    BEGIN
#undef PARAM_VEC3
#define PARAM_VEC3(xmlattr,m) PARAM_ON(xmlattr,m)
#include "Scene3D.Qt3DWrapper.cpp"
#undef PARAM_VEC3
#define PARAM_VEC3(xmlattr,m) PARAM_OFF
    END
}

void Scene3D::setVector4Parameter(int id, std::string param, double x, double y, double z, double w)
{
    Qt3DCore::QNode *node = nodeById(id);

    LOG_QT3D_CALL((boost::format("%x set %s = <%f, %f, %f, %f>") % (void*)node % param % x % y % z % w).str());

    QQuaternion quaternion(w, x, y, z);

    BEGIN
#undef PARAM_VEC4
#define PARAM_VEC4(xmlattr,m) PARAM_ON(xmlattr,m)
#include "Scene3D.Qt3DWrapper.cpp"
#undef PARAM_VEC4
#define PARAM_VEC4(xmlattr,m) PARAM_OFF
    END
}

#undef BEGIN
#undef END
#undef TYPE
#undef ENDTYPE
#undef PARAM_INT
#undef PARAM_FLOAT
#undef PARAM_STRING
#undef PARAM_VEC2
#undef PARAM_VEC3
#undef PARAM_VEC4
#undef PARAM_ON
#undef PARAM_OFF

