#ifndef UIPROXY_H_INCLUDED
#define UIPROXY_H_INCLUDED

#include <QObject>
#include <QString>
#include <QLayout>

#include "tinyxml2.h"

using tinyxml2::XMLElement;

class UIProxy : public QObject
{
    Q_OBJECT

public:
    UIProxy(QObject *parent = 0);
    virtual ~UIProxy();

protected:
    QWidget * createStuff(QWidget *parent, XMLElement *e);

public slots:
    void onCreateWindow(QString xml);
};

#endif // UIPROXY_H_INCLUDED

