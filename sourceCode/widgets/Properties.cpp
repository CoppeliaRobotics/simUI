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

    if(role == Qt::DecorationRole && column == 0 && row < icons.size())
        return icons[row];

    if(role == Qt::EditRole || role == Qt::DisplayRole)
    {
        if(column == 0) return pnames[row];
        if(column == 1) return ptypes[row];
        if(column == 2) return pvalues[row];
    }

    if(row < pflags.size())
    {
        int pf = pflags[row];
        if(pf == -1)
        {
            if(role == Qt::ForegroundRole)
                return qwidget->palette().color(QPalette::WindowText);
            if(role == Qt::BackgroundRole)
                return qwidget->palette().color(QPalette::Window);
        }
        else if(pf == -3 && column == 2 && role == Qt::ForegroundRole)
        {
            return qwidget->palette().color(QPalette::Disabled, QPalette::WindowText);
        }
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

void CustomTableModel::setRow(int row, const QString &pname, const QString &ptype, const QString &pvalue, int pflags, const QString &pdisplayk, const QString &pdisplayv, int icon)
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
    if(row < this->icons.size())
        this->icons[row] = icon;

    endResetModel();
}

void CustomTableModel::setRows(const QStringList &pnames, const QStringList &ptypes, const QStringList &pvalues, QList<int> pflags, const QStringList &pdisplayk, const QStringList &pdisplayv, const QList<int> &icons)
{
    beginResetModel();

    this->pnames = pnames;
    this->ptypes = ptypes;
    this->pvalues = pvalues;
    this->pflags = pflags;
    this->pdisplayk = pdisplayk;
    this->pdisplayv = pdisplayv;
    this->icons = icons;

    endResetModel();
}

Qt::ItemFlags CustomTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);

    if(!index.isValid())
        return defaultFlags;

    int row = index.row();
    int column = index.column();

    if(row >= 0 && row < pflags.size())
    {
        int pf = pflags[row];
        if(column == 2 && pf >= 0 && (pf & 3) == 0)
            return defaultFlags | Qt::ItemIsEditable;
        if(column == 0 && pf == -1)
            return defaultFlags & ~Qt::ItemIsSelectable;
    }

    return defaultFlags;
}

class OverlayIconDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    OverlayIconDelegate(QObject* parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        QStyledItemDelegate::paint(painter, option, index);

        int icon = index.model()->data(index, Qt::DecorationRole).toInt();
        if(icon > 0)
        {
            const int sz = 8;
            const int szq = 2;
            int x = option.rect.left() + (option.rect.height() - sz) / 2;
            int y = option.rect.top() + (option.rect.height() - sz) / 2;
            painter->setPen(Qt::black);
            painter->fillRect(x + 1, y + 1, sz - 2, sz - 2, Qt::white);
            painter->drawRect(x + 0, y + 0, sz, sz);

            if(icon == 1 || icon == 2)
                painter->drawLine(x + szq, y + 2 * szq, x + 3 * szq, y + 2 * szq);
            if(icon == 2)
                painter->drawLine(x + 2 * szq, y + szq, x + 2 * szq, y + 3 * szq);
        }

        auto tableView = dynamic_cast<const QTableView*>(option.widget);
        if(tableView)
        {
            auto model = dynamic_cast<CustomTableModel*>(tableView->model());
            if(model && index.row() >= 0 && index.row() < model->pflags.size())
            {
                int pflags = model->pflags[index.row()];
                if(pflags == -1)
                {
                    painter->save();
                    painter->setPen(QPen(QColor(180, 180, 180), 1));
                    painter->drawLine(option.rect.bottomLeft() + QPoint(0, 1), option.rect.bottomRight() + QPoint(0, 1));
                    painter->restore();
                }
            }
        }
    }

    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override
    {
        if(event->type() == QEvent::MouseButtonRelease)
        {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            int icon = index.model()->data(index, Qt::DecorationRole).toInt();
            if(icon > 0 && mouseEvent->pos().x() < option.rect.left() + option.rect.height())
            {
                emit iconClicked(index);
                return true;
            }
        }

        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

signals:
    void iconClicked(const QModelIndex &index);
};

#include "Properties.moc"

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
    QFont font = tableView->font();
    font.setPointSizeF(font.pointSizeF() * 0.85);
    tableView->setFont(font);
    tableView->resizeRowsToContents();
    auto iconDelegate = new OverlayIconDelegate(tableView);
    QObject::connect(iconDelegate, &OverlayIconDelegate::iconClicked, [=] (const QModelIndex &index) {
        if(index.column() != 2)
            ui->onPropertiesDoubleClick(this, index);
    });
    tableView->setItemDelegate(iconDelegate);

    QObject::connect(tableView->selectionModel(), &QItemSelectionModel::selectionChanged, ui, &UI::onPropertiesSelectionChange);
    QObject::connect(tableView, &QAbstractItemView::doubleClicked, ui, [=] (const QModelIndex &index) {
        if(index.column() != 2)
            ui->onPropertiesDoubleClick(this, index);
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

inline void setColumnSpan(QTableView *tableView, int row, int columnSpan)
{
    int oldColumnSpan = tableView->columnSpan(row, 0);
    if(oldColumnSpan != columnSpan)
        tableView->setSpan(row, 0, 1, columnSpan);
}

void Properties::setItems(std::vector<std::string> pnames, std::vector<std::string> ptypes, std::vector<std::string> pvalues, std::vector<int> pflags, std::vector<std::string> pdisplayk, std::vector<std::string> pdisplayv, std::vector<int> icons, bool suppressSignals)
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

    model->setRows(v(pnames), v(ptypes), v(pvalues), QList<int>::fromVector(QVector<int>(pflags.begin(), pflags.end())), v(pdisplayk), v(pdisplayv), QList<int>::fromVector(QVector<int>(icons.begin(), icons.end())));
    tableView->resizeRowsToContents();
    for(size_t i = 0; i < pflags.size(); i++)
        setColumnSpan(tableView, i, pflags[i] == -1 ? 3 : 1);
}

void Properties::setRow(int row, std::string pname, std::string ptype, std::string pvalue, int pflags, std::string pdisplayk, std::string pdisplayv, int icon, bool suppressSignals)
{
    QTableView *tableView = static_cast<QTableView*>(getQWidget());
    auto idx = tableView->currentIndex();
    CustomTableModel *model = static_cast<CustomTableModel*>(tableView->model());
    model->setRow(row, QString::fromStdString(pname), QString::fromStdString(ptype), QString::fromStdString(pvalue), pflags, QString::fromStdString(pdisplayk), QString::fromStdString(pdisplayv), icon);
    setColumnSpan(tableView, row, pflags == -1 ? 3 : 1);
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
            if(cmKeys[i][0] == '#')
                a->setEnabled(false);
            else
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

