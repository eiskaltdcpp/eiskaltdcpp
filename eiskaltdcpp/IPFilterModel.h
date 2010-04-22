#ifndef IPFILTERMODEL_H
#define IPFILTERMODEL_H

#include <QAbstractItemModel>
#include <QHash>

#define COLUMN_RULE_NAME        0
#define COLUMN_RULE_DIRECTION   1

class IPFilterModelItem{

public:

    IPFilterModelItem(const QList<QVariant> &data, IPFilterModelItem *parent = NULL);
    ~IPFilterModelItem();

    void appendChild(IPFilterModelItem *child);

    IPFilterModelItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    IPFilterModelItem *parent();

    QList<IPFilterModelItem*> childItems;

    void updateColumn(int, QVariant);

private:
    QList<QVariant> itemData;
    IPFilterModelItem *parentItem;
};

class IPFilterModel : public QAbstractItemModel{
Q_OBJECT

public:
    IPFilterModel(QObject *parent = 0);
    ~IPFilterModel();

    /** */
    QVariant data(const QModelIndex &, int) const;
    /** */
    Qt::ItemFlags flags(const QModelIndex &) const;
    /** */
    QVariant headerData(int section, Qt::Orientation, int role = Qt::DisplayRole) const;
    /** */
    QModelIndex index(int, int, const QModelIndex &parent = QModelIndex()) const;
    /** */
    QModelIndex parent(const QModelIndex &index) const;
    /** */
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    /** */
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    /** sort list */
    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    /** */
    void addResult(const QString &, const QString &);

    /** */
    void clearModel();

    /** */
    void repaint();

    /** */
    void moveUp(const QModelIndex &);
    /** */
    void moveDown(const QModelIndex &);

    /** */
    void removeItem(IPFilterModelItem*);

private:

    /** */
    void moveIndex(const QModelIndex &, bool down);

    QHash<QString, IPFilterModelItem*> rules;
    IPFilterModelItem *rootItem;
};

#endif // IPFILTERMODEL_H
