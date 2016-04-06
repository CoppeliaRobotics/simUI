#include "UIProxy.h"

#include <QThread>
#include <QPushButton>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QWidget>
#include <QLabel>
#include <QGroupBox>
#include <QDialog>

#include <iostream>

#include "v_repLib.h"

using namespace tinyxml2;

UIProxy::UIProxy(QObject *parent)
    : QObject(parent)
{
}

UIProxy::~UIProxy()
{
}

QWidget * UIProxy::createStuff(int scriptID, QWidget *parent, XMLElement *e)
{
    std::string tag(e->Value());
    if(tag == "button")
    {
        int id;
        if(e->QueryIntAttribute("id", &id) != XML_NO_ERROR) id = nextId++;

        const char *text = e->Attribute("text");
        if(!text) text = "???";

        QPushButton *button = new QPushButton(text, parent);

        connect(button, SIGNAL(released()), this, SLOT(onButtonClick()));

        objectById[id] = button;
        objectId[button] = id;

        const char *onclick = e->Attribute("onclick");
        if(onclick)
        {
            onclickCallback[id] = LuaCallbackFunction(std::string(onclick), scriptID);
        }

        return button;
    }
    else if(tag == "label")
    {
        int id;
        if(e->QueryIntAttribute("id", &id) != XML_NO_ERROR) id = nextId++;

        const char *text = e->Attribute("text");
        if(!text) text = "???";

        QLabel *label = new QLabel(text, parent);

        objectById[id] = label;
        objectId[label] = id;

        return label;
    }
    else if(tag == "group")
    {
        int id;
        if(e->QueryIntAttribute("id", &id) != XML_NO_ERROR) id = nextId++;

        const char *text = e->Attribute("text");

        const char *layoutName = e->Attribute("layout");
        std::string name(layoutName);

        QGroupBox *groupBox = new QGroupBox(text);

        if(name == "vbox" || name == "hbox")
        {
            QBoxLayout *layout1 = NULL;
            if(name == "vbox") layout1 = new QVBoxLayout();
            if(name == "hbox") layout1 = new QHBoxLayout();
            for(XMLElement *e1 = e->FirstChildElement(); e1; e1 = e1->NextSiblingElement())
            {
                QWidget *w1 = createStuff(scriptID, groupBox, e1);
                layout1->addWidget(w1);
            }
            groupBox->setLayout(layout1);
        }
        else if(name == "grid")
        {
            QGridLayout *layout1 = new QGridLayout();
            int row = 0;
            int col = 0;
            for(XMLElement *e1 = e->FirstChildElement(); e1; e1 = e1->NextSiblingElement())
            {
                std::string tag1(e1->Value());
                if(tag1 == "br")
                {
                    col = 0;
                    row++;
                    continue;
                }
                QWidget *w1 = createStuff(scriptID, groupBox, e1);
                layout1->addWidget(w1, row, col);
                col++;
            }
            groupBox->setLayout(layout1);
        }
        else if(name == "form")
        {
            QFormLayout *layout1 = new QFormLayout();
            QWidget *lastLabel = NULL;
            for(XMLElement *e1 = e->FirstChildElement(); e1; e1 = e1->NextSiblingElement())
            {
                if(lastLabel == NULL)
                {
                    lastLabel = createStuff(scriptID, groupBox, e1);
                }
                else
                {
                    QWidget *w1 = createStuff(scriptID, groupBox, e1);
                    layout1->addRow(lastLabel, w1);
                    lastLabel = NULL;
                }
            }
            groupBox->setLayout(layout1);
        }
        else
        {
            std::cout << "error: bad layout: " << name << std::endl;
        }

        objectById[id] = groupBox;
        objectId[groupBox] = id;
        return groupBox;
    }
    else
    {
        return NULL;
    }
}

void UIProxy::onCreate(int scriptID, QString xml)
{
    QWidget *mainWindow = (QWidget *)simGetMainWindow(1);
    QDialog *w = new QDialog(mainWindow, Qt::Tool);

    XMLDocument *doc = new XMLDocument;
    std::string xmlStr = xml.toStdString();
    XMLError err = doc->Parse(xmlStr.c_str(), xmlStr.size());
    if(err != XML_NO_ERROR)
    {
        std::cout << "XML error " << err << std::endl;
        delete doc;
        return;
    }
    XMLElement *root = doc->FirstChildElement();
    if(root->NextSiblingElement())
    {
        std::cout << "XML error: must have exactly one root element" << std::endl;
        delete doc;
        return;
    }
    QVBoxLayout *layout = new QVBoxLayout();
    QWidget *w1 = createStuff(scriptID, w, root);
    delete doc;
    layout->addWidget(w1);
    w->setLayout(layout);
    w->setWindowTitle("Custom UI");
    w->show();
}

void UIProxy::onButtonClick()
{
    try
    {
        emit buttonClick(objectId[sender()]);
    }
    catch(std::exception& ex)
    {
    }
}

void UIProxy::onValueChange()
{
    try
    {
        emit valueChange(objectId[sender()]);
    }
    catch(std::exception& ex)
    {
    }
}

