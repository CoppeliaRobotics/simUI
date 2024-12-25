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

class PropertiesWidget;

class Properties : public Widget, public EventOnKeyPress, EventOnDoubleClick
{
protected:
    std::vector<std::string> pnames;
    std::vector<std::string> ptypes;
    std::vector<std::string> pvalues;
    std::vector<std::string> cmKeys;
    std::vector<std::string> cmTitles;
    std::string onSelectionChange;
    std::string onContextMenuTriggered;

public:
    Properties();
    virtual ~Properties();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UI *ui, QWidget *parent);
    void setSelection(int row, bool suppressSignals);
    void setItems(std::vector<std::string> pnames, std::vector<std::string> ptypes, std::vector<std::string> pvalues, bool suppressSignals);
    void setRow(int row, std::string pname, std::string ptype, std::string pvalue, bool suppressSignals);
    void setContextMenu(std::vector<std::string> keys, std::vector<std::string> titles);
    void fillContextMenu(PropertiesWidget *owner, QMenu *menu);
    inline bool hasContextMenu() { return cmKeys.size() > 0; }
    friend class SIM;
};

class PropertiesWidget : public QTableView
{
    Q_OBJECT
private:
    Properties *properties;

public:
    PropertiesWidget(Properties *properties_, QWidget *parent);
    void keyPressEvent(QKeyEvent *event);

signals:
    void contextMenuTriggered(std::string key);
};

class CustomTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    CustomTableModel(PropertiesWidget *w, QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void setRow(int row, const QString &pname, const QString &ptype, const QString &pvalue);
    void setRows(const QStringList &column1, const QStringList &column2, const QStringList &column3);

    inline PropertiesWidget * getQWidget() { return qwidget; }

private:
    QList<QStringList> tableData;
    PropertiesWidget *qwidget;
};

#endif // PROPERTIES_H_INCLUDED

