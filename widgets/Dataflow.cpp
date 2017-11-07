#include "Dataflow.h"

#include "XMLUtils.h"

#include "UIProxy.h"
#include "debug.h"

#include <stdexcept>

#include <boost/foreach.hpp>

std::map<QDataflowModel*, Dataflow*> Dataflow::byModel_;

Dataflow::Dataflow()
    : Widget("dataflow"), nextId_(1)
{
}

Dataflow::~Dataflow()
{
    QDataflowCanvas *canvas = dynamic_cast<QDataflowCanvas*>(getQWidget());
    if(canvas)
        byModel_.erase(canvas->model());
}

Dataflow * Dataflow::byModel(QDataflowModel *model)
{
    std::map<QDataflowModel*, Dataflow*>::iterator it = byModel_.find(model);
    if(it == byModel_.end()) return 0;
    else return it->second;
}

static bool isValidColor(const std::vector<int>& c)
{
    for(int i = 0; i < 3; i++)
        if(c[i] < 0 || c[i] > 255)
            return false;
    return true;
}

static QColor toQColor(const std::vector<int>& c)
{
    return QColor(c[0], c[1], c[2]);
}

void Dataflow::parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(parent, widgets, e);

    background_color = xmlutils::getAttrIntV(e, "background-color", "-1,-1,-1", 3, 3, ",");

    object_color = xmlutils::getAttrIntV(e, "object-color", "-1,-1,-1", 3, 3, ",");

    header_color = xmlutils::getAttrIntV(e, "header-color", "-1,-1,-1", 3, 3, ",");

    border_color = xmlutils::getAttrIntV(e, "border-color", "-1,-1,-1", 3, 3, ",");

    iolet_color = xmlutils::getAttrIntV(e, "iolet-color", "-1,-1,-1", 3, 3, ",");

    connection_color = xmlutils::getAttrIntV(e, "connection-color", "-1,-1,-1", 3, 3, ",");

    selection_color = xmlutils::getAttrIntV(e, "selection-color", "-1,-1,-1", 3, 3, ",");

    hilight_color = xmlutils::getAttrIntV(e, "hilight-color", "-1,-1,-1", 3, 3, ",");

    hover_color = xmlutils::getAttrIntV(e, "hover-color", "-1,-1,-1", 3, 3, ",");

    onNodeAdded = xmlutils::getAttrStr(e, "on-node-added", "");

    onNodeRemoved = xmlutils::getAttrStr(e, "on-node-removed", "");

    onNodeValidChanged = xmlutils::getAttrStr(e, "on-node-valid-changed", "");

    onNodePosChanged = xmlutils::getAttrStr(e, "on-node-pos-changed", "");

    onNodeTextChanged = xmlutils::getAttrStr(e, "on-node-text-changed", "");

    onNodeInletCountChanged = xmlutils::getAttrStr(e, "on-node-inlet-count-changed", "");

    onNodeOutletCountChanged = xmlutils::getAttrStr(e, "on-node-outlet-count-changed", "");

    onConnectionAdded = xmlutils::getAttrStr(e, "on-connection-added", "");

    onConnectionRemoved = xmlutils::getAttrStr(e, "on-connection-removed", "");
}

QWidget * Dataflow::createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent)
{
    QDataflowCanvas *dataflow = new QDataflowCanvas(parent);
    dataflow->setEnabled(enabled);
    dataflow->setVisible(visible);
    dataflow->setStyleSheet(QString::fromStdString(style));
    dataflow->setMinimumSize(QSize(400, 400));
    QColor bgcol(dataflow->palette().color(dataflow->backgroundRole()));
    if(isValidColor(background_color))
        bgcol = toQColor(background_color);
    //dataflow->setBackground(QBrush(bgcol));
    QDataflowModel *model = dataflow->model();
    QObject::connect(model, &QDataflowModel::nodeAdded, uiproxy, &UIProxy::onNodeAdded);
    QObject::connect(model, &QDataflowModel::nodeRemoved, uiproxy, &UIProxy::onNodeRemoved);
    QObject::connect(model, &QDataflowModel::nodeValidChanged, uiproxy, &UIProxy::onNodeValidChanged);
    QObject::connect(model, &QDataflowModel::nodePosChanged, uiproxy, &UIProxy::onNodePosChanged);
    QObject::connect(model, &QDataflowModel::nodeTextChanged, uiproxy, &UIProxy::onNodeTextChanged);
    QObject::connect(model, &QDataflowModel::nodeInletCountChanged, uiproxy, &UIProxy::onNodeInletCountChanged);
    QObject::connect(model, &QDataflowModel::nodeOutletCountChanged, uiproxy, &UIProxy::onNodeOutletCountChanged);
    QObject::connect(model, &QDataflowModel::connectionAdded, uiproxy, &UIProxy::onConnectionAdded);
    QObject::connect(model, &QDataflowModel::connectionRemoved, uiproxy, &UIProxy::onConnectionRemoved);
    setQWidget(dataflow);
    setProxy(proxy);
    byModel_[model] = this;
    return dataflow;
}

int Dataflow::nextId()
{
    return nextId_++;
}

void Dataflow::mapNode(QDataflowModelNode *node, int id)
{
    nodeById[id] = node;
    nodeId[node] = id;
}

void Dataflow::unmapNode(QDataflowModelNode *node)
{
    int id = nodeId[node];
    nodeById.erase(id);
    nodeId.erase(node);
}

QDataflowModelNode * Dataflow::getNode(int id)
{
    std::map<int, QDataflowModelNode*>::iterator it = findNode(id);
    if(it == nodeById.end())
    {
        throw std::range_error("invalid node id");
    }
    return it->second;
}

int Dataflow::getNodeId(QDataflowModelNode *node)
{
    std::map<QDataflowModelNode*, int>::iterator it = nodeId.find(node);
    if(it == nodeId.end())
    {
        throw std::range_error("invalid node (has no mapped id)");
    }
    return it->second;
}

std::map<int, QDataflowModelNode*>::iterator Dataflow::findNode(int id)
{
    return nodeById.find(id);
}

void Dataflow::addNode(int id, QPoint pos, QString text, int inlets, int outlets)
{
    QDataflowModelNode *node = model()->create(pos, text, inlets, outlets);
    mapNode(node, id);
}

void Dataflow::removeNode(int id)
{
    QDataflowModelNode *node = getNode(id);
    model()->remove(node);
    unmapNode(node);
}

void Dataflow::setNodeValid(int id, bool valid)
{
    QDataflowModelNode *node = getNode(id);
    node->setValid(valid);
}

bool Dataflow::isNodeValid(int id)
{
    QDataflowModelNode *node = getNode(id);
    return node->isValid();
}

void Dataflow::setNodePos(int id, QPoint pos)
{
    QDataflowModelNode *node = getNode(id);
    node->setPos(pos);
}

QPoint Dataflow::getNodePos(int id)
{
    QDataflowModelNode *node = getNode(id);
    return node->pos();
}

void Dataflow::setNodeText(int id, QString text)
{
    QDataflowModelNode *node = getNode(id);
    node->setText(text);
}

std::string Dataflow::getNodeText(int id)
{
    QDataflowModelNode *node = getNode(id);
    return node->text().toStdString();
}

void Dataflow::setNodeInletCount(int id, int count)
{
    QDataflowModelNode *node = getNode(id);
    node->setInletCount(count);
}

int Dataflow::getNodeInletCount(int id)
{
    QDataflowModelNode *node = getNode(id);
    return node->inletCount();
}

void Dataflow::setNodeOutletCount(int id, int count)
{
    QDataflowModelNode *node = getNode(id);
    node->setOutletCount(count);
}

int Dataflow::getNodeOutletCount(int id)
{
    QDataflowModelNode *node = getNode(id);
    return node->outletCount();
}

void Dataflow::addConnection(int srcNodeId, int srcOutlet, int dstNodeId, int dstInlet)
{
    QDataflowModelNode *src = getNode(srcNodeId), *dst = getNode(dstNodeId);
    model()->connect(src, srcOutlet, dst, dstInlet);
}

void Dataflow::removeConnection(int srcNodeId, int srcOutlet, int dstNodeId, int dstInlet)
{
    QDataflowModelNode *src = getNode(srcNodeId), *dst = getNode(dstNodeId);
    model()->disconnect(src, srcOutlet, dst, dstInlet);
}

