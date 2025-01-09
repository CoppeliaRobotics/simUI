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
    return tableData.isEmpty() ? 0 : tableData.first().size();
}

int CustomTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3; // We have 3 columns
}

QVariant CustomTableModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid() || role != Qt::DisplayRole)
    {
        return QVariant();
    }

    int row = index.row();
    int column = index.column();

    if(column >= tableData.size() || row >= tableData.first().size())
    {
        return QVariant();
    }

    return tableData[column][row];
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

void CustomTableModel::setRow(int row, const QString &column1, const QString &column2, const QString &column3)
{
    beginResetModel();

    tableData[0][row] = column1;
    tableData[1][row] = column2;
    tableData[2][row] = column3;

    endResetModel();
}

void CustomTableModel::setRows(const QStringList &column1, const QStringList &column2, const QStringList &column3)
{
    beginResetModel();

    tableData.clear();
    tableData.append(column1);
    tableData.append(column2);
    tableData.append(column3);

    endResetModel();
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
}

QWidget * Properties::createQtWidget(Proxy *proxy, UI *ui, QWidget *parent)
{
    PropertiesWidget *tableView = new PropertiesWidget(this, parent);
    tableView->setEnabled(enabled);
    tableView->setVisible(visible);
    tableView->setStyleSheet(QString::fromStdString(style));
    tableView->horizontalHeader()->setVisible(true);
    tableView->verticalHeader()->setVisible(false);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setModel(new CustomTableModel(tableView, tableView));
    tableView->setShowGrid(true);
    tableView->setWordWrap(false);

    QObject::connect(tableView->selectionModel(), &QItemSelectionModel::selectionChanged, ui, &UI::onPropertiesSelectionChange);
    QObject::connect(tableView, &QAbstractItemView::doubleClicked, ui, &UI::onPropertiesDoubleClick);
    QObject::connect(tableView, &PropertiesWidget::contextMenuTriggered, ui, [=] (std::string key) {
        ui->propertiesContextMenuTriggered(this, key);
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

void Properties::setItems(std::vector<std::string> pnames, std::vector<std::string> ptypes, std::vector<std::string> pvalues, bool suppressSignals)
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

    model->setRows(v(pnames), v(ptypes), v(pvalues));
}

void Properties::setRow(int row, std::string pname, std::string ptype, std::string pvalue, bool suppressSignals)
{
    QTableView *tableView = static_cast<QTableView*>(getQWidget());
    CustomTableModel *model = static_cast<CustomTableModel*>(tableView->model());
    model->setRow(row, QString::fromStdString(pname), QString::fromStdString(ptype), QString::fromStdString(pvalue));
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

