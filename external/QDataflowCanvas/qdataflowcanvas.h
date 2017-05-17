/* QDataflowCanvas - a dataflow widget for Qt
 * Copyright (C) 2017 Federico Ferri
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef QDATAFLOWCANVAS_H
#define QDATAFLOWCANVAS_H

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QMouseEvent>
#include <QLineEdit>

#include "qdataflowmodel.h"

class QDataflowNode;
class QDataflowInlet;
class QDataflowOutlet;
class QDataflowConnection;
class QDataflowTextCompletion;
class QDataflowNodeTextLabel;

enum QDataflowItemType {
    QDataflowItemTypeNode = QGraphicsItem::UserType + 1,
    QDataflowItemTypeConnection = QGraphicsItem::UserType + 2,
    QDataflowItemTypeInlet = QGraphicsItem::UserType + 3,
    QDataflowItemTypeOutlet = QGraphicsItem::UserType + 4
};

class QDataflowCanvas : public QGraphicsView
{
    Q_OBJECT
public:
    QDataflowCanvas(QWidget *parent);
    virtual ~QDataflowCanvas();

    QDataflowModel * model();
    void setModel(QDataflowModel *model);

    QDataflowNode * node(QDataflowModelNode *node);
    QDataflowConnection * connection(QDataflowModelConnection *conn);

    QList<QDataflowNode*> selectedNodes();
    QList<QDataflowConnection*> selectedConnections();

    bool isSomeNodeInEditMode();

    QDataflowTextCompletion * completion() const {return completion_;}
    void setCompletion(QDataflowTextCompletion *completion) {completion_ = completion;}

    void raiseItem(QGraphicsItem *item);

protected:
    template<typename T>
    T * itemAtT(const QPointF &point);

protected slots:
    void mouseDoubleClickEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void itemTextEditorTextChange();
    void onNodeAdded(QDataflowModelNode *mdlnode);
    void onNodeRemoved(QDataflowModelNode *mdlnode);
    void onNodeValidChanged(QDataflowModelNode *mdlnode, bool valid);
    void onNodePosChanged(QDataflowModelNode *mdlnode, QPoint pos);
    void onNodeTextChanged(QDataflowModelNode *mdlnode, QString text);
    void onNodeInletCountChanged(QDataflowModelNode *mdlnode, int count);
    void onNodeOutletCountChanged(QDataflowModelNode *mdlnode, int count);
    void onConnectionAdded(QDataflowModelConnection *mdlconn);
    void onConnectionRemoved(QDataflowModelConnection *mdlconn);

    friend class QDataflowNode;
    friend class QDataflowIOlet;
    friend class QDataflowInlet;
    friend class QDataflowOutlet;
    friend class QDataflowConnection;

private:
    QDataflowModel *model_;
    QDataflowTextCompletion *completion_;
    QSet<QDataflowNode*> ownedNodes_;
    QSet<QDataflowConnection*> ownedConnections_;
    QMap<QDataflowModelNode*, QDataflowNode*> nodes_;
    QMap<QDataflowModelConnection*, QDataflowConnection*> connections_;
};

class QDataflowNode : public QGraphicsItem
{
protected:
    QDataflowNode(QDataflowCanvas *canvas_, QDataflowModelNode *modelNode);

public:
    QDataflowModelNode * modelNode() const;

    QDataflowInlet * inlet(int index) const {return inlets_.at(index);}
    int inletCount() const {return inlets_.size();}
    void setInletCount(int count, bool skipAdjust = false);
    QDataflowOutlet * outlet(int index) const {return outlets_.at(index);}
    int outletCount() const {return outlets_.size();}
    void setOutletCount(int count, bool skipAdjust = false);

    int type() const {return QDataflowItemTypeNode;}

    void setText(QString text);
    QString text() const;

    void setValid(bool valid);
    bool isValid() const;

    void adjustConnections() const;

    QRectF boundingRect() const;

    void adjust();

    QDataflowCanvas * canvas() const {return canvas_;}

    qreal ioletWidth() const {return 10;}
    qreal ioletHeight() const {return 3;}
    qreal ioletSpacing() const {return 13;}
    qreal inletsWidth() const;
    qreal outletsWidth() const;
    QPen objectPen() const;
    QBrush objectBrush() const;
    QBrush headerBrush() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void enterEditMode();
    void exitEditMode(bool revertText);
    bool isInEditMode() const;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

private:
    QDataflowCanvas *canvas_;
    QDataflowModelNode *modelNode_;
    QList<QDataflowInlet*> inlets_;
    QList<QDataflowOutlet*> outlets_;
    QGraphicsRectItem *inputHeader_;
    QGraphicsRectItem *objectBox_;
    QGraphicsRectItem *outputHeader_;
    QDataflowNodeTextLabel *textItem_;
    bool valid_;
    QString oldText_;

    friend class QDataflowCanvas;
};

class QDataflowIOlet : public QGraphicsItem
{
protected:
    QDataflowIOlet(QDataflowNode *node, int index);

public:
    virtual int type() const = 0;

    QDataflowNode * node() const {return node_;}
    int index() const {return index_;}

    void addConnection(QDataflowConnection *connection);
    void removeConnection(QDataflowConnection *connection);
    QList<QDataflowConnection*> connections() const;
    void adjustConnections() const;

    QDataflowCanvas * canvas() const {return canvas_;}

protected:
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QDataflowCanvas *canvas_;
    QList<QDataflowConnection*> connections_;
    QDataflowNode *node_;
    int index_;

    friend class QDataflowCanvas;
    friend class QDataflowNode;
};

class QDataflowInlet : public QDataflowIOlet
{
protected:
    QDataflowInlet(QDataflowNode *node, int index);

public:
    int type() const {return QDataflowItemTypeInlet;}

    void onDataRecevied(void *data);

    friend class QDataflowCanvas;
    friend class QDataflowNode;
};

class QDataflowOutlet : public QDataflowIOlet
{
protected:
    QDataflowOutlet(QDataflowNode *node, int index);

public:
    int type() const {return QDataflowItemTypeOutlet;}

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

private:
    QGraphicsLineItem *tmpConn_;

    friend class QDataflowCanvas;
    friend class QDataflowNode;
};

class QDataflowConnection : public QGraphicsItem
{
protected:
    QDataflowConnection(QDataflowCanvas *canvas, QDataflowModelConnection *modelConnection);

public:
    QDataflowModelConnection * modelConnection() const;

    QDataflowOutlet * source() const {return source_;}
    QDataflowInlet * dest() const {return dest_;}

    void adjust();

    QDataflowCanvas * canvas() const {return canvas_;}

    int type() const {return QDataflowItemTypeConnection;}

protected:
    QRectF boundingRect() const;
    QPainterPath shape() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QDataflowCanvas *canvas_;
    QDataflowModelConnection *modelConnection_;
    QDataflowOutlet *source_;
    QDataflowInlet *dest_;
    QPointF sourcePoint_;
    QPointF destPoint_;

    friend class QDataflowCanvas;
    friend class QDataflowInlet;
    friend class QDataflowOutlet;
};

class QDataflowNodeTextLabel : public QGraphicsTextItem
{
protected:
    QDataflowNodeTextLabel(QDataflowNode *node, QGraphicsItem *parent);

public:
    bool sceneEvent(QEvent *event);
    void setCompletion(QStringList list);
    void clearCompletion();
    void acceptCompletion();
    void cycleCompletion(int d);
    void updateCompletion();
    void complete();

private:
    QDataflowNode *node_;
    QList<QGraphicsSimpleTextItem*> completionItems_;
    QList<QGraphicsRectItem*> completionRectItems_;
    int completionIndex_;
    bool completionActive_;

    friend class QDataflowNode;
};

class QDataflowTextCompletion
{
public:
    virtual void complete(QString nodeText, QStringList &completionList);
};

template<typename T>
T * QDataflowCanvas::itemAtT(const QPointF &point)
{
    foreach(QGraphicsItem *item, scene()->items(point, Qt::IntersectsItemShape, Qt::DescendingOrder, transform()))
    {
         if(T *itemT = dynamic_cast<T*>(item))
             return itemT;
    }
    return 0;
}


#endif // QDATAFLOWCANVAS_H
