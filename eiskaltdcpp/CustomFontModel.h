#ifndef CUSTOMFONTMODEL_H
#define CUSTOMFONTMODEL_H

#include <QAbstractItemModel>

#define COLUMN_CUSTOM_FONT_DESC     0
#define COLUMN_CUSTOM_FONT_DISP     1

class CustomFontItem{
public:
    CustomFontItem(const QList<QVariant> &data, CustomFontItem *parent = 0);
    virtual ~CustomFontItem();

    void appendChild(CustomFontItem *child);

    CustomFontItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    CustomFontItem *parent();
    void updateColumn(int column, QVariant var);

    QList<CustomFontItem*> childItems;

    QString key;
    QString custom_font;
private:

    QList<QVariant> itemData;
    CustomFontItem *parentItem;
};

class CustomFontModel : public QAbstractItemModel
{
Q_OBJECT
public:
    explicit CustomFontModel(QObject *parent = 0);
    virtual ~CustomFontModel();

    /** */
    QVariant data(const QModelIndex &, int) const;
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

public slots:
    void itemDoubleClicked(const QModelIndex&);
    void ok();

private:
    void addNewFont(const QString &wkey, const QString &desc);

    CustomFontItem *rootItem;
};

#endif // CUSTOMFONTMODEL_H
