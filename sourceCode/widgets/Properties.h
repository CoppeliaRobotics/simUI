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
    std::vector<std::string> cmKeys;
    std::vector<std::string> cmTitles;

    std::string onSelectionChange;
    std::string onContextMenuTriggered;
    std::string onPropertyEdit;

public:
    Properties();
    virtual ~Properties();

    void parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e);
    QWidget * createQtWidget(Proxy *proxy, UI *ui, QWidget *parent);
    void setSelection(int row, bool suppressSignals);
    void setItems(std::vector<std::string> pnames, std::vector<std::string> ptypes, std::vector<std::string> pvalues, std::vector<int> pflags, std::vector<std::string> pdisplayk, std::vector<std::string> pdisplayv, bool suppressSignals);
    void setRow(int row, std::string pname, std::string ptype, std::string pvalue, int pflags, std::string pdisplayk, std::string pdisplayv, bool suppressSignals);
    void setContextMenu(std::vector<std::string> keys, std::vector<std::string> titles);
    void fillContextMenu(PropertiesWidget *owner, QMenu *menu);
    inline bool hasContextMenu() { return cmKeys.size() > 0; }
    std::string saveState();
    bool restoreState(std::string state);

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
    void propertyEditRequest(std::string key, std::string value);
};

class CustomTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    CustomTableModel(PropertiesWidget *w, QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void setRow(int row, const QString &pname, const QString &ptype, const QString &pvalue, int pflags, const QString &pdisplayk, const QString &pdisplayv);
    void setRows(const QStringList &pnames, const QStringList &ptypes, const QStringList &pvalues, QList<int> pflags, const QStringList &pdisplayk, const QStringList &pdisplayv);
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    inline PropertiesWidget * getQWidget() { return qwidget; }

private:
    QStringList pnames;
    QStringList ptypes;
    QStringList pvalues;
    QList<int> pflags;
    QStringList pdisplayk;
    QStringList pdisplayv;
    PropertiesWidget *qwidget;
};

#endif // PROPERTIES_H_INCLUDED

