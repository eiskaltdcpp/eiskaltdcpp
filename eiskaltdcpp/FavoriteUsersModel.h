#ifndef FAVORITEUSERSMODEL_H
#define FAVORITEUSERSMODEL_H

#include <QAbstractItemModel>
#include <QHash>

#define COLUMN_USER_NICK        0
#define COLUMN_USER_HOST        1
#define COLUMN_USER_SEEN        2
#define COLUMN_USER_DESC        3

class FavoriteUserItem{

public:

    FavoriteUserItem(const QList<QVariant> &data, FavoriteUserItem *parent = NULL);
    ~FavoriteUserItem();

    void appendChild(FavoriteUserItem *child);

    FavoriteUserItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    FavoriteUserItem *parent();

    QList<FavoriteUserItem*> childItems;

    void updateColumn(unsigned, QVariant);
    QString cid;

private:
    QList<QVariant> itemData;
    FavoriteUserItem *parentItem;
};

class FavoriteUsersModel : public QAbstractItemModel
{
Q_OBJECT
public:
    typedef QMap<QString, QVariant> VarMap;

    explicit FavoriteUsersModel(QObject *parent = 0);
    virtual ~FavoriteUsersModel();

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

    void addUser(const VarMap&);
    void updateUserStatus(const QString &, const QString &);
    void removeUser(const QString &);

    FavoriteUserItem *itemForCID(const QString &cid){
        if (itemHash.contains(cid))
            return itemHash.value(cid);

        return NULL;
    }

    void repaint();
private:
    Qt::SortOrder sortOrder;
    int sortColumn;

    QHash<QString, FavoriteUserItem*> itemHash;

    FavoriteUserItem *rootItem;
};

#endif // FAVORITEUSERSMODEL_H
