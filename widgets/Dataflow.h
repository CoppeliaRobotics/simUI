#ifndef DATAFLOW_H_INCLUDED
#define DATAFLOW_H_INCLUDED

#include "config.h"

#include <vector>
#include <map>
#include <string>

#include <QWidget>

#include "tinyxml2.h"

class Proxy;
class UIProxy;

#include "Widget.h"

#include "qdataflowcanvas.h"

class Dataflow : public Widget
{
protected:
    std::vector<int> background_color;
    std::vector<int> object_color;
    std::vector<int> header_color;
    std::vector<int> border_color;
    std::vector<int> iolet_color;
    std::vector<int> connection_color;
    std::vector<int> selection_color;
    std::vector<int> hilight_color;
    std::vector<int> hover_color;
    std::string onNodeAdded;
    std::string onNodeRemoved;
    std::string onNodeValidChanged;
    std::string onNodePosChanged;
    std::string onNodeTextChanged;
    std::string onNodeInletCountChanged;
    std::string onNodeOutletCountChanged;
    std::string onConnectionAdded;
    std::string onConnectionRemoved;

    std::map<int, QDataflowModelNode*> nodeById;
    std::map<QDataflowModelNode*, int> nodeId;

    int nextId_;

    static std::map<QDataflowModel*, Dataflow*> byModel_;

public:
    Dataflow();
    virtual ~Dataflow();

    static Dataflow * byModel(QDataflowModel *model);

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UIProxy *uiproxy, QWidget *parent);

    inline QDataflowCanvas * canvas() {return static_cast<QDataflowCanvas*>(getQWidget());}
    inline QDataflowModel * model() {return canvas()->model();}

    int nextId();
    QDataflowModelNode * getNode(int id);
    int getNodeId(QDataflowModelNode *node);
    void mapNode(QDataflowModelNode *node, int id);
    void unmapNode(QDataflowModelNode *node);
    std::map<int, QDataflowModelNode*>::iterator findNode(int id);

    void addNode(int id, QPoint pos, QString text, int inlets, int outlets);
    void removeNode(int id);
    void setNodeValid(int id, bool valid);
    bool isNodeValid(int id);
    void setNodePos(int id, QPoint pos);
    QPoint getNodePos(int id);
    void setNodeText(int id, QString text);
    std::string getNodeText(int id);
    void setNodeInletCount(int id, int count);
    int getNodeInletCount(int id);
    void setNodeOutletCount(int id, int count);
    int getNodeOutletCount(int id);
    void addConnection(int srcNodeId, int srcOutlet, int dstNodeId, int dstOutlet);
    void removeConnection(int srcNodeId, int srcOutlet, int dstNodeId, int dstOutlet);

    friend class UIFunctions;
};

#endif // DATAFLOW_H_INCLUDED

