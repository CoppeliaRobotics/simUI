#include "UIProxy.h"

#include <QThread>
#include <QPushButton>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QSlider>
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
    else if(tag == "edit")
    {
        int id;
        if(e->QueryIntAttribute("id", &id) != XML_NO_ERROR) id = nextId++;

        const char *text = e->Attribute("text");
        if(!text) text = "???";

        QLineEdit *edit = new QLineEdit(parent);

        connect(edit, SIGNAL(textChanged(QString)), this, SLOT(onValueChange(QString)));

        objectById[id] = edit;
        objectId[edit] = id;

        const char *onchange = e->Attribute("onchange");
        if(onchange)
        {
            onchangeCallback[id] = LuaCallbackFunction(std::string(onchange), scriptID);
        }

        return edit;
    }
    else if(tag == "hslider" || tag == "vslider")
    {
        int id;
        if(e->QueryIntAttribute("id", &id) != XML_NO_ERROR) id = nextId++;

        const char *text = e->Attribute("text");
        if(!text) text = "???";

        QSlider *slider = new QSlider(tag == "hslider" ? Qt::Horizontal : Qt::Vertical, parent);

        int minimum;
        if(e->QueryIntAttribute("minimum", &minimum) == XML_NO_ERROR)
            slider->setMinimum(minimum);

        int maximum;
        if(e->QueryIntAttribute("maximum", &maximum) == XML_NO_ERROR)
            slider->setMaximum(maximum);

        connect(slider, SIGNAL(valueChanged(int)), this, SLOT(onValueChange(int)));

        objectById[id] = slider;
        objectId[slider] = id;

        const char *onchange = e->Attribute("onchange");
        if(onchange)
        {
            onchangeCallback[id] = LuaCallbackFunction(std::string(onchange), scriptID);
        }

        return slider;
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

        QGroupBox *groupBox = new QGroupBox(text, parent);

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
    w->setWindowFlags(Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
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

void UIProxy::onValueChange(int value)
{
    try
    {
        emit valueChange(objectId[sender()], value);
    }
    catch(std::exception& ex)
    {
    }
}

void UIProxy::onValueChange(QString value)
{
    try
    {
        emit valueChange(objectId[sender()], value);
    }
    catch(std::exception& ex)
    {
    }
}

