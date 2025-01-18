#include "Properties.h"

#include "XMLUtils.h"

#include "UI.h"

#include <iostream>
#include <boost/lexical_cast.hpp>

#include <QKeyEvent>
#include <QHeaderView>
#include <QTableView>
#include <QAbstractTableModel>

CustomTableModel::CustomTableModel(PropertiesWidget *w, QObject *parent)
    : QAbstractTableModel(parent), qwidget(w) {}

int CustomTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return std::min(std::min(pnames.size(), ptypes.size()), pvalues.size());
}

int CustomTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3; // We have 3 columns
}

QVariant CustomTableModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    int row = index.row();
    int column = index.column();

    if(column >= columnCount() || row >= rowCount())
        return QVariant();

    if(role == Qt::DisplayRole)
    {
        if(column == 0 && row < pdisplayk.size())
            return pdisplayk[row];
        if(column == 2 && row < pdisplayv.size())
            return pdisplayv[row];
    }

    if(role == Qt::EditRole || role == Qt::DisplayRole)
    {
        if(column == 0) return pnames[row];
        if(column == 1) return ptypes[row];
        if(column == 2) return pvalues[row];
    }

    return QVariant();
}

bool CustomTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(index.isValid() && index.column() == 2)
        qwidget->propertyEditRequest(pnames[index.row()].toStdString(), value.toString().toStdString());
    return false;
}

QVariant CustomTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role != Qt::DisplayRole)
    {
        return QVariant();
    }

    if(orientation == Qt::Horizontal)
    {
        switch(section)
        {
            case 0:
                return QString("Name");
            case 1:
                return QString("Type");
            case 2:
                return QString("Value");
            default:
                return QVariant();
        }
    }
    else
    {
        return QString("Row %1").arg(section + 1);
    }
}

void CustomTableModel::setRow(int row, const QString &pname, const QString &ptype, const QString &pvalue, int pflags, const QString &pdisplayk, const QString &pdisplayv)
{
    beginResetModel();

    if(row < rowCount())
    {
        this->pnames[row] = pname;
        this->ptypes[row] = ptype;
        this->pvalues[row] = pvalue;
    }
    if(row < this->pflags.size())
        this->pflags[row] = pflags;
    if(row < this->pdisplayk.size())
        this->pdisplayk[row] = pdisplayk;
    if(row < this->pdisplayv.size())
        this->pdisplayv[row] = pdisplayv;

    endResetModel();
}

void CustomTableModel::setRows(const QStringList &pnames, const QStringList &ptypes, const QStringList &pvalues, QList<int> pflags, const QStringList &pdisplayk, const QStringList &pdisplayv)
{
    beginResetModel();

    this->pnames = pnames;
    this->ptypes = ptypes;
    this->pvalues = pvalues;
    this->pflags = pflags;
    this->pdisplayk = pdisplayk;
    this->pdisplayv = pdisplayv;

    endResetModel();
}

Qt::ItemFlags CustomTableModel::flags(const QModelIndex &index) const
{
    if(!index.isValid())
        return QAbstractTableModel::flags(index);

    int row = index.row();
    int column = index.column();

    if(column == 2 && row < pflags.size() && pflags[row] >= 0 && (pflags[row] & 3) == 0)
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    else
        return QAbstractTableModel::flags(index);
}

Properties::Properties()
    : Widget("properties")
{
}

Properties::~Properties()
{
}

void Properties::parse(Widget *parent, std::map<int, Widget*>& widgets, tinyxml2::XMLElement *e)
{
    Widget::parse(parent, widgets, e);

    onSelectionChange = xmlutils::getAttrStr(e, "on-selection-change", "");

    onKeyPress = xmlutils::getAttrStr(e, "on-key-press", "");

    ondoubleclick = xmlutils::getAttrStr(e, "on-double-click", "");

    onContextMenuTriggered = xmlutils::getAttrStr(e, "on-context-menu-triggered", "");

    onPropertyEdit = xmlutils::getAttrStr(e, "on-property-edit", "");
}

QWidget * Properties::createQtWidget(Proxy *proxy, UI *ui, QWidget *parent)
{
    PropertiesWidget *tableView = new PropertiesWidget(this, parent);
    tableView->setEnabled(enabled);
    tableView->setVisible(visible);
    tableView->setStyleSheet(QString::fromStdString(style));
    tableView->horizontalHeader()->setVisible(true);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->verticalHeader()->setVisible(false);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setModel(new CustomTableModel(tableView, tableView));
    tableView->setShowGrid(true);
    tableView->setWordWrap(false);

    QObject::connect(tableView->selectionModel(), &QItemSelectionModel::selectionChanged, ui, &UI::onPropertiesSelectionChange);
    QObject::connect(tableView, &QAbstractItemView::doubleClicked, ui, [=] (const QModelIndex &index) {
        if(index.column() != 2)
            ui->onPropertiesDoubleClick(index);
    });
    QObject::connect(tableView, &PropertiesWidget::contextMenuTriggered, ui, [=] (std::string key) {
        ui->propertiesContextMenuTriggered(this, key);
    });
    QObject::connect(tableView, &PropertiesWidget::propertyEditRequest, ui, [=] (std::string key, std::string value) {
        ui->propertiesPropertyEdit(this, key, value);
    });

    setQWidget(tableView);
    setProxy(proxy);

    return tableView;
}

void Properties::setSelection(int row, bool suppressSignals)
{
    QTableView *tableView = static_cast<QTableView*>(getQWidget());
    bool oldSignalsState = tableView->blockSignals(suppressSignals);
    tableView->setCurrentIndex(tableView->model()->index(row, 0));
    tableView->blockSignals(oldSignalsState);
}

void Properties::setItems(std::vector<std::string> pnames, std::vector<std::string> ptypes, std::vector<std::string> pvalues, std::vector<int> pflags, std::vector<std::string> pdisplayk, std::vector<std::string> pdisplayv, bool suppressSignals)
{
    QTableView *tableView = static_cast<QTableView*>(getQWidget());
    CustomTableModel *model = static_cast<CustomTableModel*>(tableView->model());

    auto v = [](const std::vector<std::string>& vec) -> QStringList {
        QStringList qStringList;
        std::for_each(vec.begin(), vec.end(), [&qStringList](const std::string& str) {
            qStringList.append(QString::fromStdString(str));
        });
        return qStringList;
    };

    model->setRows(v(pnames), v(ptypes), v(pvalues), QList<int>::fromVector(QVector<int>(pflags.begin(), pflags.end())), v(pdisplayk), v(pdisplayv));
}

void Properties::setRow(int row, std::string pname, std::string ptype, std::string pvalue, int pflags, std::string pdisplayk, std::string pdisplayv, bool suppressSignals)
{
    QTableView *tableView = static_cast<QTableView*>(getQWidget());
    auto idx = tableView->currentIndex();
    CustomTableModel *model = static_cast<CustomTableModel*>(tableView->model());
    model->setRow(row, QString::fromStdString(pname), QString::fromStdString(ptype), QString::fromStdString(pvalue), pflags, QString::fromStdString(pdisplayk), QString::fromStdString(pdisplayv));
    bool oldSignalsState = tableView->blockSignals(true);
    tableView->setCurrentIndex(idx);
    tableView->blockSignals(oldSignalsState);
}

void Properties::setContextMenu(std::vector<std::string> keys, std::vector<std::string> titles)
{
    cmKeys = keys;
    cmTitles = titles;
}

void Properties::fillContextMenu(PropertiesWidget *owner, QMenu *menu)
{
    for(int i = 0; i < cmKeys.size(); i++) {
        if(cmKeys[i] == "--")
        {
            menu->addSeparator();
        }
        else
        {
            QAction *a = menu->addAction(QString::fromStdString(i < cmTitles.size() ? cmTitles[i] : cmKeys[i]));
            QObject::connect(a, &QAction::triggered, owner, [=] () { owner->contextMenuTriggered(cmKeys[i]); });
        }
    }
}

std::string Properties::saveState()
{
    QTableWidget *tablewidget = static_cast<QTableWidget*>(getQWidget());
    QByteArray hhState = tablewidget->horizontalHeader()->saveState();
    std::string hh(hhState.constData(), hhState.length());
    return hh;
}

bool Properties::restoreState(std::string state)
{
    QTableWidget *tablewidget = static_cast<QTableWidget*>(getQWidget());
    QByteArray hhState(state.data(), state.size());
    bool ret = true;
    ret = ret && tablewidget->horizontalHeader()->restoreState(hhState);
    return ret;
}

PropertiesWidget::PropertiesWidget(Properties *properties_, QWidget *parent)
    : QTableView(parent), properties(properties_)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTableWidget::customContextMenuRequested, this, [=] (const QPoint &pos) {
        if(!properties->hasContextMenu()) return;
        QMenu contextMenu(this);
        properties->fillContextMenu(this, &contextMenu);
        /*QAction *selectedAction = */contextMenu.exec(viewport()->mapToGlobal(pos));
    });
}

void PropertiesWidget::keyPressEvent(QKeyEvent *event)
{
    UI::getInstance()->keyPressed(properties, event->key(), event->text().toStdString());
    QTableView::keyPressEvent(event);
}

