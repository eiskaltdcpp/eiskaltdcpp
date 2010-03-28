#ifndef FINISHEDTRANSFERSMODEL_H
#define FINISHEDTRANSFERSMODEL_H

#include <QAbstractItemModel>
#include <QHash>
#include <QMap>

#define COLUMN_FINISHED_NAME    0
#define COLUMN_FINISHED_PATH    1
#define COLUMN_FINISHED_TIME    2
#define COLUMN_FINISHED_USER    3
#define COLUMN_FINISHED_TR      4
#define COLUMN_FINISHED_SPEED   5
#define COLUMN_FINISHED_CRC32   6
#define COLUMN_FINISHED_TARGET  7
#define COLUMN_FINISHED_ELAPS   8

class FinishedTransfersItem
{

public:
    FinishedTransfersItem(const QList<QVariant> &data, FinishedTransfersItem *parent = 0);
    ~FinishedTransfersItem();

    void appendChild(FinishedTransfersItem *child);

    FinishedTransfersItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    FinishedTransfersItem *parent() const;
    void updateColumn(int column, QVariant var);

    QList<FinishedTransfersItem*> childItems;

private:
    QList<QVariant> itemData;
    FinishedTransfersItem *parentItem;
};

class FinishedTransfersModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    enum ViewType{
        FileView=0,
        UserView
    };

    FinishedTransfersModel(QObject *parent = 0);
    ~FinishedTransfersModel();

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
    virtual void sort() { sort(sortColumn, sortOrder); }

    /** Clear model and redraw view*/
    void clearModel();

    /** */
    void repaint();

    /** */
    void addFile(QMap<QString, QVariant> params);
    /** */
    void addUser(QMap<QString, QVariant> params);
    /** */
    void remFile(QString file);
    /** */
    void remUser(QString cid);

    /** */
    void switchViewType(ViewType);
private:
    /** */
    FinishedTransfersItem *findFile(const QString &fname);
    /** */
    FinishedTransfersItem *findUser(const QString &cid);

    FinishedTransfersItem *rootItem;
    FinishedTransfersItem *fileItem;
    FinishedTransfersItem *userItem;

    int sortColumn;
    Qt::SortOrder sortOrder;

    QHash<QString, FinishedTransfersItem* > file_hash;
    QHash<QString, FinishedTransfersItem* > user_hash;
    QMap<int, QString> file_header_table;
    QMap<int, QString> user_header_table;
};

#endif // FINISHEDTRANSFERSMODEL_H
