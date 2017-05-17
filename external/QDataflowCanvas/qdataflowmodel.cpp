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
#include "qdataflowmodel.h"
#include "qdataflowcanvas.h"

QDataflowModel::QDataflowModel(QObject *parent)
    : QObject(parent)
{

}

void QDataflowModel::addNode(QDataflowModelNode *node)
{
    if(!node) return;
    if(nodes_.contains(node)) return;
    node->setParent(this);
    nodes_.insert(node);
    QObject::connect(node, &QDataflowModelNode::validChanged, this, &QDataflowModel::onValidChanged);
    QObject::connect(node, &QDataflowModelNode::posChanged, this, &QDataflowModel::onPosChanged);
    QObject::connect(node, &QDataflowModelNode::textChanged, this, &QDataflowModel::onTextChanged);
    QObject::connect(node, &QDataflowModelNode::inletCountChanged, this, &QDataflowModel::onInletCountChanged);
    QObject::connect(node, &QDataflowModelNode::outletCountChanged, this, &QDataflowModel::onOutletCountChanged);
    emit nodeAdded(node);
}

void QDataflowModel::removeNode(QDataflowModelNode *node)
{
    if(!node) return;
    if(!nodes_.contains(node)) return;
    foreach(QDataflowModelInlet *inlet, node->inlets())
        foreach(QDataflowModelConnection *conn, inlet->connections())
            removeConnection(conn);
    foreach(QDataflowModelOutlet *outlet, node->outlets())
        foreach(QDataflowModelConnection *conn, outlet->connections())
            removeConnection(conn);
    QObject::disconnect(node, &QDataflowModelNode::validChanged, this, &QDataflowModel::onValidChanged);
    QObject::disconnect(node, &QDataflowModelNode::posChanged, this, &QDataflowModel::onPosChanged);
    QObject::disconnect(node, &QDataflowModelNode::textChanged, this, &QDataflowModel::onTextChanged);
    QObject::disconnect(node, &QDataflowModelNode::inletCountChanged, this, &QDataflowModel::onInletCountChanged);
    QObject::disconnect(node, &QDataflowModelNode::outletCountChanged, this, &QDataflowModel::onOutletCountChanged);
    nodes_.remove(node);
    emit nodeRemoved(node);
}

void QDataflowModel::addConnection(QDataflowModelConnection *conn)
{
    if(!conn) return;
    if(!findConnections(conn).isEmpty()) return;
    conn->setParent(this);
    connections_.insert(conn);
    emit connectionAdded(conn);
}

void QDataflowModel::addConnection(QDataflowModelOutlet *outlet, QDataflowModelInlet *inlet)
{
    if(!outlet || !inlet) return;
    if(!findConnections(outlet, inlet).isEmpty()) return;
    addConnection(new QDataflowModelConnection(this, outlet, inlet));
}

void QDataflowModel::addConnection(QDataflowModelNode *sourceNode, int sourceOutlet, QDataflowModelNode *destNode, int destInlet)
{
    if(!sourceNode || !destNode) return;
    if(!findConnections(sourceNode, sourceOutlet, destNode, destInlet).isEmpty()) return;
    addConnection(new QDataflowModelConnection(this, sourceNode->outlet(sourceOutlet), destNode->inlet(destInlet)));
}

void QDataflowModel::removeConnection(QDataflowModelConnection *conn)
{
    if(!conn) return;
    if(!connections_.contains(conn)) return;
    connections_.remove(conn);
    emit connectionRemoved(conn);
}

void QDataflowModel::removeConnection(QDataflowModelOutlet *outlet, QDataflowModelInlet *inlet)
{
    if(!outlet || !inlet) return;
    foreach(QDataflowModelConnection *conn, findConnections(outlet, inlet))
    {
        removeConnection(conn);
    }
}

void QDataflowModel::removeConnection(QDataflowModelNode *sourceNode, int sourceOutlet, QDataflowModelNode *destNode, int destInlet)
{
    if(!sourceNode || !destNode) return;
    foreach(QDataflowModelConnection *conn, findConnections(sourceNode, sourceOutlet, destNode, destInlet))
    {
        removeConnection(conn);
    }
}

QList<QDataflowModelConnection*> QDataflowModel::findConnections(QDataflowModelConnection *conn) const
{
    if(!conn) return QList<QDataflowModelConnection*>();
    return findConnections(conn->source(), conn->dest());
}

QList<QDataflowModelConnection*> QDataflowModel::findConnections(QDataflowModelOutlet *source, QDataflowModelInlet *dest) const
{
    if(!source || !dest) return QList<QDataflowModelConnection*>();
    return findConnections(source->node(), source->index(), dest->node(), dest->index());
}

QList<QDataflowModelConnection*> QDataflowModel::findConnections(QDataflowModelNode *sourceNode, int sourceOutlet, QDataflowModelNode *destNode, int destInlet) const
{
    if(!sourceNode || !destNode) return QList<QDataflowModelConnection*>();
    QList<QDataflowModelConnection*> ret;
    foreach(QDataflowModelConnection *conn, connections_)
    {
        QDataflowModelOutlet *src = conn->source();
        QDataflowModelInlet *dst = conn->dest();
        if(src->node() == sourceNode && src->index() == sourceOutlet && dst->node() == destNode && dst->index() == destInlet)
            ret.push_back(conn);
    }
    return ret;
}

void QDataflowModel::onValidChanged(bool valid)
{
    if(QDataflowModelNode *node = dynamic_cast<QDataflowModelNode*>(sender()))
        emit nodeValidChanged(node, valid);
}

void QDataflowModel::onPosChanged(QPoint pos)
{
    if(QDataflowModelNode *node = dynamic_cast<QDataflowModelNode*>(sender()))
        emit nodePosChanged(node, pos);
}

void QDataflowModel::onTextChanged(QString text)
{
    if(QDataflowModelNode *node = dynamic_cast<QDataflowModelNode*>(sender()))
        emit nodeTextChanged(node, text);
}

void QDataflowModel::onInletCountChanged(int count)
{
    if(QDataflowModelNode *node = dynamic_cast<QDataflowModelNode*>(sender()))
        emit nodeInletCountChanged(node, count);
}

void QDataflowModel::onOutletCountChanged(int count)
{
    if(QDataflowModelNode *node = dynamic_cast<QDataflowModelNode*>(sender()))
        emit nodeOutletCountChanged(node, count);
}

QDataflowModelNode::QDataflowModelNode(QDataflowModel *parent, QPoint pos, QString text, int inletCount, int outletCount)
    : QObject(parent), valid_(false), pos_(pos), text_(text), dataflowMetaObject_(0L)
{
    for(int i = 0; i < inletCount; i++) addInlet();
    for(int i = 0; i < outletCount; i++) addOutlet();
}

QDataflowModel * QDataflowModelNode::model()
{
    return static_cast<QDataflowModel*>(parent());
}

QDataflowMetaObject * QDataflowModelNode::dataflowMetaObject() const
{
    return dataflowMetaObject_;
}

void QDataflowModelNode::setDataflowMetaObject(QDataflowMetaObject *dataflowMetaObject)
{
    if(dataflowMetaObject_)
        delete dataflowMetaObject_;

    dataflowMetaObject_ = dataflowMetaObject;

    if(dataflowMetaObject_)
        dataflowMetaObject_->node_ = this;
}

bool QDataflowModelNode::isValid() const
{
    return valid_;
}

QPoint QDataflowModelNode::pos() const
{
    return pos_;
}

QString QDataflowModelNode::text() const
{
    return text_;
}

QList<QDataflowModelInlet*> QDataflowModelNode::inlets() const
{
    return inlets_;
}

QDataflowModelInlet * QDataflowModelNode::inlet(int index) const
{
    if(index >= 0 && index < inlets_.length())
        return inlets_[index];
    else
        return 0L;
}

int QDataflowModelNode::inletCount() const
{
    return inlets_.length();
}

QList<QDataflowModelOutlet*> QDataflowModelNode::outlets() const
{
    return outlets_;
}

QDataflowModelOutlet * QDataflowModelNode::outlet(int index) const
{
    if(index >= 0 && index < outlets_.length())
        return outlets_[index];
    else
        return 0L;
}

int QDataflowModelNode::outletCount() const
{
    return outlets_.length();
}

void QDataflowModelNode::setValid(bool valid)
{
    valid_ = valid;
    emit validChanged(valid);
}

void QDataflowModelNode::setPos(QPoint pos)
{
    pos_ = pos;
    emit posChanged(pos);
}

void QDataflowModelNode::setText(const QString &text)
{
    text_ = text;
    emit textChanged(text);
}

void QDataflowModelNode::addInlet()
{
    addInlet(new QDataflowModelInlet(this, inletCount()));
}

void QDataflowModelNode::removeLastInlet()
{
    if(inlets_.isEmpty()) return;
    QDataflowModelInlet *inlet = inlets_.back();
    foreach(QDataflowModelConnection *conn, inlet->connections())
        model()->removeConnection(conn);
    inlets_.pop_back();
    emit inletCountChanged(inletCount());
}

void QDataflowModelNode::setInletCount(int count)
{
    if(inletCount() == count) return;

    bool shouldBlockSignals = blockSignals(true);

    while(inletCount() < count)
        addInlet();

    while(inletCount() > count)
        removeLastInlet();

    blockSignals(shouldBlockSignals);

    emit inletCountChanged(count);
}

void QDataflowModelNode::addOutlet()
{
    addOutlet(new QDataflowModelOutlet(this, outletCount()));
}

void QDataflowModelNode::removeLastOutlet()
{
    if(outlets_.isEmpty()) return;
    QDataflowModelOutlet *outlet = outlets_.back();
    foreach(QDataflowModelConnection *conn, outlet->connections())
        model()->removeConnection(conn);
    outlets_.pop_back();
    emit outletCountChanged(outletCount());
}

void QDataflowModelNode::setOutletCount(int count)
{
    if(outletCount() == count) return;

    bool shouldBlockSignals = blockSignals(true);

    while(outletCount() < count)
        addOutlet();

    while(outletCount() > count)
        removeLastOutlet();

    blockSignals(shouldBlockSignals);

    emit outletCountChanged(count);
}

void QDataflowModelNode::addInlet(QDataflowModelInlet *inlet)
{
    if(!inlet) return;
    inlet->setParent(this);
    inlets_.append(inlet);
    emit inletCountChanged(inletCount());
}

void QDataflowModelNode::addOutlet(QDataflowModelOutlet *outlet)
{
    if(!outlet) return;
    outlet->setParent(this);
    outlets_.append(outlet);
    emit outletCountChanged(outletCount());
}

QDebug operator<<(QDebug debug, const QDataflowModelNode &node)
{
    QDebugStateSaver stateSaver(debug);
    debug.nospace() << "QDataflowModelNode";
    debug.nospace() << "(" << reinterpret_cast<const void*>(&node) <<
                       ", text=" << node.text() << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const QDataflowModelNode *node)
{
    return debug << *node;
}

QDataflowModelIOlet::QDataflowModelIOlet(QDataflowModelNode *parent, int index)
    : QObject(parent), index_(index)
{

}

QDataflowModel * QDataflowModelIOlet::model()
{
    return node()->model();
}

QDataflowModelNode * QDataflowModelIOlet::node() const
{
    return static_cast<QDataflowModelNode*>(parent());
}

int QDataflowModelIOlet::index() const
{
    return index_;
}

void QDataflowModelIOlet::addConnection(QDataflowModelConnection *conn)
{
    connections_.push_back(conn);
}

QList<QDataflowModelConnection*> QDataflowModelIOlet::connections() const
{
    return connections_;
}

QDataflowModelInlet::QDataflowModelInlet(QDataflowModelNode *parent, int index)
    : QDataflowModelIOlet(parent, index)
{

}

QDebug operator<<(QDebug debug, const QDataflowModelInlet &inlet)
{
    QDebugStateSaver stateSaver(debug);
    debug.nospace() << "QDataflowModelInlet";
    debug.nospace() << "(" << reinterpret_cast<const void*>(&inlet) <<
                       ", node=" << inlet.node() << ", index=" << inlet.index() << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const QDataflowModelInlet *inlet)
{
    return debug << *inlet;
}

QDataflowModelOutlet::QDataflowModelOutlet(QDataflowModelNode *parent, int index)
    : QDataflowModelIOlet(parent, index)
{

}

QDebug operator<<(QDebug debug, const QDataflowModelOutlet &outlet)
{
    QDebugStateSaver stateSaver(debug);
    debug.nospace() << "QDataflowModelOutlet";
    debug.nospace() << "(" << reinterpret_cast<const void*>(&outlet) <<
                       ", node=" << outlet.node() << ", index=" << outlet.index() << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const QDataflowModelOutlet *outlet)
{
    return debug << *outlet;
}

QDataflowModelConnection::QDataflowModelConnection(QDataflowModel *parent, QDataflowModelOutlet *source, QDataflowModelInlet *dest)
    : QObject(parent), source_(source), dest_(dest)
{
    source->addConnection(this);
    dest->addConnection(this);
}

QDataflowModel * QDataflowModelConnection::model()
{
    return static_cast<QDataflowModel*>(parent());
}

QDataflowModelOutlet * QDataflowModelConnection::source() const
{
    return source_;
}

QDataflowModelInlet * QDataflowModelConnection::dest() const
{
    return dest_;
}

QDebug operator<<(QDebug debug, const QDataflowModelConnection &conn)
{
    QDebugStateSaver stateSaver(debug);
    debug.nospace() << "QDataflowModelConnection";
    debug.nospace() << "(" << reinterpret_cast<const void*>(&conn) <<
                       ", src=" << conn.source() << ", dst=" << conn.dest() << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const QDataflowModelConnection *conn)
{
    return debug << *conn;
}

bool QDataflowMetaObject::init(QStringList args)
{
    Q_UNUSED(args);

    return true;
}

void QDataflowMetaObject::onDataReceved(int inlet, void *data)
{
    Q_UNUSED(inlet);
    Q_UNUSED(data);
}

void QDataflowMetaObject::sendData(int outletIndex, void *data)
{
    foreach(QDataflowModelConnection *conn, outlet(outletIndex)->connections())
    {
        QDataflowMetaObject *mo = conn->dest()->node()->dataflowMetaObject();
        if(mo)
            mo->onDataReceved(conn->dest()->index(), data);
    }
}

QDataflowModelDebugSignals::QDataflowModelDebugSignals(QDataflowModel *parent)
    : QObject(parent)
{
    QObject::connect(parent, &QDataflowModel::nodeAdded, this, &QDataflowModelDebugSignals::onNodeAdded);
    QObject::connect(parent, &QDataflowModel::nodeRemoved, this, &QDataflowModelDebugSignals::onNodeRemoved);
    QObject::connect(parent, &QDataflowModel::nodePosChanged, this, &QDataflowModelDebugSignals::onNodePosChanged);
    QObject::connect(parent, &QDataflowModel::nodeTextChanged, this, &QDataflowModelDebugSignals::onNodeTextChanged);
    QObject::connect(parent, &QDataflowModel::nodeInletCountChanged, this, &QDataflowModelDebugSignals::onNodeInletCountChanged);
    QObject::connect(parent, &QDataflowModel::nodeOutletCountChanged, this, &QDataflowModelDebugSignals::onNodeOutletCountChanged);
    QObject::connect(parent, &QDataflowModel::connectionAdded, this, &QDataflowModelDebugSignals::onConnectionAdded);
    QObject::connect(parent, &QDataflowModel::connectionRemoved, this, &QDataflowModelDebugSignals::onConnectionRemoved);
}

QDebug QDataflowModelDebugSignals::debug() const
{
    return qDebug() << parent();
}

void QDataflowModelDebugSignals::onNodeAdded(QDataflowModelNode *node)
{
    debug() << "nodeAdded" << node;
}

void QDataflowModelDebugSignals::onNodeRemoved(QDataflowModelNode *node)
{
    debug() << "nodeRemoved" << node;
}

void QDataflowModelDebugSignals::onNodePosChanged(QDataflowModelNode *node, QPoint pos)
{
    debug() << "nodePosChanged" << node << pos;
}

void QDataflowModelDebugSignals::onNodeTextChanged(QDataflowModelNode *node, QString text)
{
    debug() << "nodeTextChanged" << node << text;
}

void QDataflowModelDebugSignals::onNodeInletCountChanged(QDataflowModelNode *node, int count)
{
    debug() << "nodeInletCountChanged" << node << count;
}

void QDataflowModelDebugSignals::onNodeOutletCountChanged(QDataflowModelNode *node, int count)
{
    debug() << "nodeOutletCountChanged" << node << count;
}

void QDataflowModelDebugSignals::onConnectionAdded(QDataflowModelConnection *conn)
{
    debug() << "connectionAdded" << conn;
}

void QDataflowModelDebugSignals::onConnectionRemoved(QDataflowModelConnection *conn)
{
    debug() << "connectionRemoved" << conn;
}
