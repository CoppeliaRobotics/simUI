#ifndef PROPERTIES_H_INCLUDED
#define PROPERTIES_H_INCLUDED

#include "config.h"

#include <vector>
#include <string>

#include <QWidget>
#include <QAbstractItemView>
#include <QTableView>

#include "tinyxml2.h"

class Proxy;
class UI;

#include "Widget.h"
#include "Event.h"

class Properties : public Widget, public EventOnKeyPress
{
protected:
    std::vector<std::string> pnames;
    std::vector<std::string> ptypes;
    std::vector<std::string> pvalues;
    std::string onCellActivate;
    std::string onSelectionChange;

public:
    Properties();
    virtual ~Properties();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UI *ui, QWidget *parent);
    void setSelection(int row, bool suppressSignals);
    void setItems(std::vector<std::string> pnames, std::vector<std::string> ptypes, std::vector<std::string> pvalues, bool suppressSignals);
    void setRow(int row, std::string pname, std::string ptype, std::string pvalue, bool suppressSignals);

    friend class SIM;
};

class PropertiesWidget : public QTableView
{
private:
    Properties *properties;

public:
    PropertiesWidget(Properties *properties_, QWidget *parent);
    void keyPressEvent(QKeyEvent *event);
};

#endif // PROPERTIES_H_INCLUDED

