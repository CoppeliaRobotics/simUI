#include "Properties.h"

#include "XMLUtils.h"

#include "UI.h"

#include <iostream>
#include <boost/lexical_cast.hpp>

#include <QKeyEvent>
#include <QHeaderView>
#include <QTableView>
#include <QAbstractTableModel>

class CustomTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    CustomTableModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void setRow(int row, const QString &pname, const QString &ptype, const QString &pvalue);
    void setRows(const QStringList &column1, const QStringList &column2, const QStringList &column3);

private:
    QList<QStringList> tableData;
};

#include "Properties.moc"

CustomTableModel::CustomTableModel(QObject *parent)
    : QAbstractTableModel(parent) {}

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

    onCellActivate = xmlutils::getAttrStr(e, "on-cell-activate", "");

    onSelectionChange = xmlutils::getAttrStr(e, "on-selection-change", "");

    onKeyPress = xmlutils::getAttrStr(e, "on-key-press", "");
}

QWidget * Properties::createQtWidget(Proxy *proxy, UI *ui, QWidget *parent)
{
    QTableView *tableView = new PropertiesWidget(this, parent);
    tableView->setEnabled(enabled);
    tableView->setVisible(visible);
    tableView->setStyleSheet(QString::fromStdString(style));
    //tableView->setRowCount(rowcount);
    //tableView->setColumnCount(columncount);
    tableView->horizontalHeader()->setVisible(true);
    tableView->verticalHeader()->setVisible(false);
    tableView->setShowGrid(true);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    //tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);

    /*
    CustomTableModel *model = new CustomTableModel;
    QStringList column1;// = {"Row1-Column1", "Row2-Column1", "Row3-Column1"};
    QStringList column2;// = {"Row1-Column2", "Row2-Column2", "Row3-Column2"};
    QStringList column3;// = {"Row1-Column3", "Row2-Column3", "Row3-Column3"};
    for(int i = 0; i < 1000; i++)
    {
        column1 << QString("Row%1-Column1").arg(i);
        column2 << QString("Row%1-Column2").arg(i);
        column3 << QString("Row%1-Column3").arg(i);
    }
    model->setRows(column1, column2, column3);
    */
    tableView->setModel(new CustomTableModel);

    //QObject::connect(tableView, &QTableView::itemSelectionChanged, ui, &UI::onTableSelectionChange);
    setQWidget(tableView);
    setProxy(proxy);

    setItems({"apple", "b"}, {"x", "y"}, {"1", "33"}, true);
    return tableView;
}

void Properties::setSelection(int row, bool suppressSignals)
{
    QTableView *tableView = static_cast<QTableView*>(getQWidget());
    bool oldSignalsState = tableView->blockSignals(suppressSignals);
    //tableView->setCurrentCell(row, column);
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

PropertiesWidget::PropertiesWidget(Properties *properties_, QWidget *parent)
    : QTableView(parent), properties(properties_)
{
}

void PropertiesWidget::keyPressEvent(QKeyEvent *event)
{
    UI::getInstance()->keyPressed(properties, event->key(), event->text().toStdString());
    QTableView::keyPressEvent(event);
}

